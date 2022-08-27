#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "common/Worker.hpp"
#include "intent/RecognitionServer.hpp"
#include "intent/IntentPerformer.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "test/Clients.hpp"

using namespace testing;
using namespace jar;

class RecognitionServerTest : public Test {
public:
    const fs::path AssetAudioPath{fs::current_path() / "asset" / "audio"};
    const std::string_view ClientHost{"127.0.0.1"};
    const std::string_view ClientPort{"8080"};
    const tcp::endpoint ServerEndpoint{net::ip::make_address("0.0.0.0"), 8080};

    RecognitionServerTest()
        : factory{std::make_shared<WitRecognitionFactory>(worker.executor())}
        , performer{IntentPerformer::create()}
        , server{RecognitionServer::create(worker.executor(), performer, factory)}
    {
    }

    void
    SetUp() override
    {
        worker.start();
    }

    void
    TearDown() override
    {
        worker.stop();
    }

public:
    Worker worker;
    WitRecognitionFactory::Ptr factory;
    IntentPerformer::Ptr performer;
    RecognitionServer::Ptr server;
};

TEST_F(RecognitionServerTest, RecognizeMessage)
{
    server->listen(ServerEndpoint);

    static const std::string_view Message{"turn off the light"};
    EXPECT_THAT(clients::recognizeMessage(ClientHost, ClientPort, Message),
                FieldsAre(IsTrue(), IsEmpty()));

    server->shutdown();
}

TEST_F(RecognitionServerTest, RecognizeSpeech)
{
    server->listen(ServerEndpoint);

    static const fs::path filePath{AssetAudioPath / "turn-off-the-light.raw"};
    EXPECT_THAT(clients::recognizeSpeech(ClientHost, ClientPort, filePath),
                FieldsAre(IsTrue(), IsEmpty()));

    server->shutdown();
}

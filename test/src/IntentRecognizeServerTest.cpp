#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tests/TestWorker.hpp"
#include "test/Clients.hpp"
#include "intent/IntentRecognizeServer.hpp"
#include "intent/IntentPerformer.hpp"
#include "intent/WitRecognitionFactory.hpp"

using namespace testing;
using namespace jar;

class IntentRecognizeServerTest : public Test {
public:
    const fs::path AssetAudioPath{fs::current_path() / "asset" / "audio"};
    const std::string_view ClientHost{"127.0.0.1"};
    const std::string_view ClientPort{"8080"};
    const tcp::endpoint ServerEndpoint{net::ip::make_address("0.0.0.0"), 8080};

    IntentRecognizeServerTest()
        : performer{IntentPerformer::create()}
        , factory{std::make_shared<WitRecognitionFactory>(worker.sslContext(), worker.executor())}
        , recognizer{std::make_shared<IntentRecognizeServer>(worker.executor(), performer, factory)}
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
    TestWorker worker;
    IntentPerformer::Ptr performer;
    WitRecognitionFactory::Ptr factory;
    IntentRecognizeServer::Ptr recognizer;
};

TEST_F(IntentRecognizeServerTest, RecognizeMessage)
{
    recognizer->listen(ServerEndpoint);

    static const std::string_view Message{"turn off the light"};
    EXPECT_THAT(clients::recognizeMessage(ClientHost, ClientPort, Message),
                FieldsAre(IsTrue(), IsEmpty()));

    recognizer->shutdown();
}

TEST_F(IntentRecognizeServerTest, RecognizeSpeech)
{
    recognizer->listen(ServerEndpoint);

    static const fs::path filePath{AssetAudioPath / "turn-off-the-light.raw"};
    EXPECT_THAT(clients::recognizeSpeech(ClientHost, ClientPort, filePath),
                FieldsAre(IsTrue(), IsEmpty()));

    recognizer->shutdown();
}

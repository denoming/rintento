#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/Worker.hpp"
#include "intent/IntentPerformer.hpp"
#include "intent/RecognitionServer.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "test/Clients.hpp"

using namespace testing;
using namespace jar;

class RecognitionServerTest : public Test {
public:
    const fs::path AssetAudioPath{fs::current_path() / "asset" / "audio"};

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
    server->listen();

    static const std::string_view Message{"turn off the light"};
    EXPECT_THAT(clients::recognizeMessage(worker.context(), kDefaultServerPort, Message),
                FieldsAre(IsTrue(), IsEmpty()));

    server->shutdown();
}

TEST_F(RecognitionServerTest, RecognizeSpeech)
{
    server->listen();

    static const fs::path filePath{AssetAudioPath / "turn-off-the-light.raw"};
    EXPECT_THAT(clients::recognizeSpeech(worker.context(), kDefaultServerPort, filePath),
                FieldsAre(IsTrue(), IsEmpty()));

    server->shutdown();
}

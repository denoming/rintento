#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/Config.hpp"
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
        : factory{std::make_shared<WitRecognitionFactory>(config, worker.executor())}
        , performer{IntentPerformer::create()}
        , server{RecognitionServer::create(worker.executor(), performer, factory)}
    {
    }

    static void
    SetUpTestSuite()
    {
        if (!config) {
            config = std::make_shared<Config>();
            ASSERT_TRUE(config->load());
        }
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
    static std::shared_ptr<Config> config;

    Worker worker;
    std::shared_ptr<WitRecognitionFactory> factory;
    std::shared_ptr<IntentPerformer> performer;
    std::shared_ptr<RecognitionServer> server;
};

std::shared_ptr<Config> RecognitionServerTest::config;

TEST_F(RecognitionServerTest, RecognizeMessage)
{
    server->listen();

    static const std::string_view Message{"turn off the light"};
    EXPECT_THAT(clients::recognizeMessage(worker.context(), kDefaultProxyServerPort, Message),
                FieldsAre(IsTrue(), IsEmpty()));

    server->shutdown();
}

TEST_F(RecognitionServerTest, RecognizeSpeech)
{
    server->listen();

    static const fs::path filePath{AssetAudioPath / "turn-off-the-light.raw"};
    EXPECT_THAT(clients::recognizeSpeech(worker.context(), kDefaultProxyServerPort, filePath),
                FieldsAre(IsTrue(), IsEmpty()));

    server->shutdown();
}

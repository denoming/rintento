#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/Config.hpp"
#include "intent/IntentPerformer.hpp"
#include "intent/RecognitionServer.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "jarvis/Worker.hpp"
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

        ASSERT_TRUE(server);
        server->listen();
    }

    void
    TearDown() override
    {
        ASSERT_TRUE(server);
        server->shutdown();

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
    static const std::string_view Message{"turn off the light"};

    auto result = clients::recognizeMessage(worker.executor(), kDefaultProxyServerPort, Message);
    EXPECT_THAT(result, FieldsAre(IsTrue(), IsEmpty()));
}

TEST_F(RecognitionServerTest, RecognizeSpeech)
{
    static const fs::path filePath{AssetAudioPath / "turn-off-the-light.raw"};

    auto result = clients::recognizeSpeech(worker.executor(), kDefaultProxyServerPort, filePath);
    EXPECT_THAT(result, FieldsAre(IsTrue(), IsEmpty()));
}

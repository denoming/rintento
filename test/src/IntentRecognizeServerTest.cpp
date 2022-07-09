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
    const tcp::endpoint Endpoint{net::ip::make_address("0.0.0.0"), 8080};

    IntentRecognizeServerTest()
        : performer{IntentPerformer::create()}
        , factory{worker.sslContext(), worker.executor()}
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
    WitRecognitionFactory factory;
    IntentRecognizeServer::Ptr recognizer;
};

TEST_F(IntentRecognizeServerTest, RecognizeMessage)
{
    recognizer->listen(Endpoint);

    EXPECT_THAT(clients::recognizeMessage("0.0.0.0", "8080", "turn off the light"),
                FieldsAre(IsTrue(), IsEmpty()));

    recognizer->shutdown();
}

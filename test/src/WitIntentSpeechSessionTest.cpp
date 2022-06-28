#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/Matchers.hpp"
#include "tests/TestWorker.hpp"
#include "intent/Config.hpp"
#include "intent/WitIntentSpeechSession.hpp"
#include "intent/WitPendingRecognition.hpp"

using namespace testing;
using namespace jar;

namespace fs = std::filesystem;

class WitIntentSpeechSessionTest : public Test {
public:
    WitIntentSpeechSessionTest()
        : session{WitIntentSpeechSession::create(worker.sslContext(), worker.executor())}
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
    WitIntentSpeechSession::Ptr session;
};

TEST_F(WitIntentSpeechSessionTest, RecognizeSpeech1)
{
    MockFunction<void(Utterances result, std::error_code error)> callback;
    EXPECT_CALL(callback, Call(Contains(isUtterance("turn off the light")), IsFalse()));
    auto pending = WitPendingRecognition::create(session, callback.AsStdFunction());

    const fs::path audioFilePath{"asset/audio/turn-off-the-light.raw"};
    session->run(WitBackendHost, WitBackendPort, WitBackendAuth, audioFilePath);

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome,
                Contains(isUtterance("turn off the light",
                                     Contains(isConfidentIntent("light_off", 0.9f)))));
}

TEST_F(WitIntentSpeechSessionTest, RecognizeSpeech2)
{
    MockFunction<void(Utterances result, std::error_code error)> callback;
    EXPECT_CALL(callback, Call(Contains(isUtterance("turn on the light")), IsFalse()));
    auto pending = WitPendingRecognition::create(session, callback.AsStdFunction());

    const fs::path audioFilePath{"asset/audio/turn-on-the-light.raw"};
    session->run(WitBackendHost, WitBackendPort, WitBackendAuth, audioFilePath);

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome,
                Contains(isUtterance("turn on the light",
                                     Contains(isConfidentIntent("light_on", 0.9f)))));
}

TEST_F(WitIntentSpeechSessionTest, CancelRecognizeSpeech)
{
    MockFunction<void(Utterances result, std::error_code error)> callback;
    EXPECT_CALL(callback, Call(_, IsTrue()));
    auto pending = WitPendingRecognition::create(session, callback.AsStdFunction());

    const fs::path audioFilePath{"asset/audio/turn-on-the-light.raw"};
    session->run(WitBackendHost, WitBackendPort, WitBackendAuth, audioFilePath);

    // Cancel message recognizing
    pending->cancel();

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_THAT(outcome, IsEmpty());
    EXPECT_EQ(error.value(), int(std::errc::operation_canceled));
}
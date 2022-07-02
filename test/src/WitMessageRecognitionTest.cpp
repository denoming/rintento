#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/Matchers.hpp"
#include "tests/TestWorker.hpp"
#include "intent/Config.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitPendingRecognition.hpp"

using namespace testing;
using namespace jar;

class WitMessageRecognitionTest : public Test {
public:
    WitMessageRecognitionTest()
        : session{WitMessageRecognition::create(worker.sslContext(), worker.executor())}
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
    WitMessageRecognition::Ptr session;
};

TEST_F(WitMessageRecognitionTest, RecognizeMessage)
{
    MockFunction<void(Utterances result, std::error_code error)> callback;
    EXPECT_CALL(callback, Call(Contains(isUtterance("turn off the light")), IsFalse()));
    auto pending = WitPendingRecognition::create(session, callback.AsStdFunction());

    std::string_view message{"turn off the light"};
    session->run(WitBackendHost, WitBackendPort, WitBackendAuth, message);

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_FALSE(error);
    EXPECT_THAT(outcome,
                Contains(isUtterance("turn off the light",
                                     Contains(isConfidentIntent("light_off", 0.9f)))));
}

TEST_F(WitMessageRecognitionTest, CancelRecognizeMessage)
{
    MockFunction<void(Utterances result, std::error_code error)> callback;
    EXPECT_CALL(callback, Call(_, IsTrue()));
    auto pending = WitPendingRecognition::create(session, callback.AsStdFunction());

    std::string_view message{"turn off the light"};
    session->run(WitBackendHost, WitBackendPort, WitBackendAuth, message);

    // Cancel message recognizing
    pending->cancel();

    std::error_code error;
    const auto outcome = pending->get(error);
    EXPECT_THAT(outcome, IsEmpty());
    EXPECT_EQ(error.value(), int(std::errc::operation_canceled));
}
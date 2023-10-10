#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "wit/Config.hpp"
#include "wit/Matchers.hpp"
#include "wit/MessageRecognition.hpp"
#include "wit/RecognitionFactory.hpp"
#include "wit/Utils.hpp"

#include <jarvisto/SecureContext.hpp>

#include <chrono>
#include <exception>

using namespace testing;
using namespace jar;

class MessageRecognitionTest : public Test {
public:
    const std::size_t kChannelCapacity{64};

    static void
    SetUpTestSuite()
    {
        ASSERT_TRUE(config.load());
    }

public:
    static wit::Config config;

    SecureContext secureContext;
    wit::RecognitionFactory factory;
};

wit::Config MessageRecognitionTest::config;

TEST_F(MessageRecognitionTest, RecognizeMessage)
{
    const std::string_view Message{"turn off the light"};

    io::io_context context{1};

    MockFunction<void(std::exception_ptr, RecognitionResult)> callback1;
    EXPECT_CALL(callback1, Call(IsFalse(), understoodIntent("light_control")));

    auto executor = context.get_executor();
    auto channel = std::make_shared<wit::MessageRecognition::Channel>(executor, 128);
    auto recognition = factory.message(executor, channel);

    /* Spawn recognition coroutine */
    io::co_spawn(
        context.get_executor(),
        [recognition]() -> io::awaitable<RecognitionResult> {
            co_return co_await recognition->run();
        },
        [&](const std::exception_ptr& eptr, RecognitionResult result) {
            callback1.Call(eptr, std::move(result));
            if (eptr) {
                context.stop();
            }
        });

    /* Spawn data provider coroutine */
    io::co_spawn(
        context.get_executor(),
        [channel]() -> io::awaitable<void> {
            std::string message = wit::messageTargetWithDate("turn off the light");
            std::ignore = co_await channel->send(io::buffer(message));
            co_await channel->send(io::error::eof);
            channel->close();
        },
        [&](const std::exception_ptr& eptr) {
            if (eptr) {
                context.stop();
            }
        });

    context.run();
}

TEST_F(MessageRecognitionTest, CancelRecognizeMessage)
{
    io::io_context context{1};

    MockFunction<void(std::exception_ptr, RecognitionResult)> callback;
    EXPECT_CALL(callback,
                Call(Truly(exceptionContainsError(Eq(sys::errc::operation_canceled))),
                     notUnderstoodIntent()));

    auto executor = context.get_executor();
    auto channel = std::make_shared<wit::MessageRecognition::Channel>(executor, kChannelCapacity);
    auto recognition = factory.message(executor, channel);

    /* Spawn recognition coroutine */
    io::co_spawn(
        context.get_executor(),
        [recognition]() -> io::awaitable<RecognitionResult> {
            co_return co_await recognition->run();
        },
        callback.AsStdFunction());

    /* Spawn cancellation timer */
    const std::chrono::milliseconds kCancelAfter{50};
    io::co_spawn(
        context.get_executor(),
        [&]() -> io::awaitable<void> {
            io::steady_timer timer{context.get_executor()};
            timer.expires_after(std::chrono::milliseconds{kCancelAfter});
            co_await timer.async_wait(io::use_awaitable);
            recognition->cancel();
        },
        io::detached);

    context.run();
}
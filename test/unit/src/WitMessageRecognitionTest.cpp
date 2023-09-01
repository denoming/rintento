#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/GeneralConfig.hpp"
#include "intent/Utils.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "test/Matchers.hpp"

#include <jarvisto/SecureContext.hpp>

#include <chrono>
#include <exception>

using namespace testing;
using namespace jar;

class WitMessageRecognitionTest : public Test {
public:
    WitMessageRecognitionTest()
        : factory{config.recognizeServerHost(),
                  config.recognizeServerPort(),
                  config.recognizeServerAuth()}
    {
    }

    static void
    SetUpTestSuite()
    {
        ASSERT_TRUE(config.load());
    }

public:
    static GeneralConfig config;

    SecureContext secureContext;
    WitRecognitionFactory factory;
};

GeneralConfig WitMessageRecognitionTest::config;

static auto
exceptionContainsError(sys::error_code errorCode)
{
    return [errorCode](const std::exception_ptr& eptr) {
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch (const sys::system_error& e) {
            return (e.code() == errorCode);
        }
        return false;
    };
}

TEST_F(WitMessageRecognitionTest, RecognizeMessage)
{
    const std::string_view Message{"turn off the light"};

    io::io_context context{1};

    MockFunction<void(std::exception_ptr, wit::Utterances)> callback1;
    EXPECT_CALL(callback1,
                Call(IsFalse(),
                     Contains(isUtterance("turn off the light",
                                          IsEmpty(),
                                          Contains(isConfidentIntent("light_off", 0.9f))))));

    auto executor = context.get_executor();
    auto channel = std::make_shared<WitMessageRecognition::Channel>(executor);
    auto recognition = factory.message(executor, channel);

    /* Spawn recognition coroutine */
    io::co_spawn(
        context.get_executor(),
        [recognition]() -> io::awaitable<wit::Utterances> {
            co_return co_await recognition->run();
        },
        [&](const std::exception_ptr& eptr, wit::Utterances result) {
            callback1.Call(eptr, std::move(result));
            if (eptr) {
                context.stop();
            }
        });

    /* Spawn data provider coroutine */
    io::co_spawn(
        context.get_executor(),
        [channel]() -> io::awaitable<void> {
            std::string data = format::messageTargetWithDate("turn off the light");
            co_await channel->async_send(sys::error_code{}, std::move(data), io::use_awaitable);
            channel->close();
        },
        [&](const std::exception_ptr& eptr) {
            if (eptr) {
                context.stop();
            }
        });

    context.run();
}

TEST_F(WitMessageRecognitionTest, CancelRecognizeMessage)
{
    io::io_context context{1};

    MockFunction<void(std::exception_ptr, wit::Utterances)> callback;
    EXPECT_CALL(
        callback,
        Call(Truly(exceptionContainsError({sys::errc::operation_canceled, sys::system_category()})),
             IsEmpty()));

    auto executor = context.get_executor();
    auto channel = std::make_shared<WitMessageRecognition::Channel>(executor);
    auto recognition = factory.message(executor, channel);
    io::co_spawn(
        context.get_executor(),
        [recognition]() -> io::awaitable<wit::Utterances> {
            co_return co_await recognition->run();
        },
        callback.AsStdFunction());

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
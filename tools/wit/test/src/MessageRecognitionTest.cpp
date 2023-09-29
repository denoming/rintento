#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "intent/GeneralConfig.hpp"
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
    MessageRecognitionTest()
        : factory{config.recognitionServerHost(),
                  config.recognitionServerPort(),
                  config.recognitionServerAuth()}
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
    wit::RecognitionFactory factory;
};

GeneralConfig MessageRecognitionTest::config;

static auto
exceptionContainsError(Matcher<int> matcher)
{
    return [matcher = std::move(matcher)](const std::exception_ptr& eptr) {
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch (const sys::system_error& e) {
            return Matches(matcher)(e.code().value());
        } catch (const std::exception& e) {
            /* Unexpected exception */
        }
        return false;
    };
}

TEST_F(MessageRecognitionTest, RecognizeMessage)
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
    auto channel = std::make_shared<wit::MessageRecognition::Channel>(executor);
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
            std::string data = wit::messageTargetWithDate("turn off the light");
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

TEST_F(MessageRecognitionTest, CancelRecognizeMessage)
{
    io::io_context context{1};

    namespace ioe = boost::asio::experimental;
    MockFunction<void(std::exception_ptr, wit::Utterances)> callback;
    EXPECT_CALL(callback,
                Call(Truly(exceptionContainsError(AnyOf(Eq(sys::errc::operation_canceled),
                                                        Eq(ioe::channel_errc::channel_cancelled)))),
                     IsEmpty()));

    auto executor = context.get_executor();
    auto channel = std::make_shared<wit::MessageRecognition::Channel>(executor);
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
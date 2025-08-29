// Copyright 2025 Denys Asauliak
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "wit/Config.hpp"
#include "wit/Matchers.hpp"
#include "wit/MessageRecognition.hpp"
#include "wit/RecognitionFactory.hpp"
#include "wit/Utils.hpp"

#include <jarvisto/network/SecureContext.hpp>

#include <chrono>
#include <exception>

using namespace testing;
using namespace jar;

class WitMessageRecognitionTest : public Test {
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

wit::Config WitMessageRecognitionTest::config;

TEST_F(WitMessageRecognitionTest, RecognizeMessage)
{
    static const std::string_view kMessage{"turn off the light in the bedroom"};

    io::io_context context{1};

    MockFunction<void(std::exception_ptr, RecognitionResult)> callback1;
    EXPECT_CALL(callback1, Call(IsFalse(), understoodIntent("light_turn_off_bedroom")));

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
            std::string message = wit::messageTargetWithDate(kMessage);
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

TEST_F(WitMessageRecognitionTest, CancelRecognizeMessage)
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
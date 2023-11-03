#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "wit/Config.hpp"
#include "wit/Matchers.hpp"
#include "wit/RecognitionFactory.hpp"
#include "wit/SpeechRecognition.hpp"

#include <jarvisto/SecureContext.hpp>
#include <sndfile.hh>

#include <chrono>
#include <exception>

using namespace testing;
using namespace jar;

namespace fs = std::filesystem;

class WitSpeechRecognitionTest : public Test {
public:
    const std::size_t kChannelCapacity{512};
    const fs::path kAssetAudioPath{fs::current_path() / "asset" / "audio"};

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

wit::Config WitSpeechRecognitionTest::config;

TEST_F(WitSpeechRecognitionTest, RecognizeSpeech)
{
    io::io_context context{1};

    MockFunction<void(std::exception_ptr, RecognitionResult)> callback1;
    EXPECT_CALL(callback1, Call(IsFalse(), understoodIntent("light_turn_off_bedroom")));

    auto executor = context.get_executor();
    auto channel = std::make_shared<wit::SpeechRecognition::Channel>(executor, kChannelCapacity);
    auto recognition = factory.speech(executor, channel);

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
    static const fs::path kAudioFilePath{kAssetAudioPath / "turn-off-light-bedroom.wav"};
    io::co_spawn(
        context.get_executor(),
        [channel]() -> io::awaitable<void> {
            SndfileHandle audioFile{kAudioFilePath};
            if (not audioFile) {
                throw std::runtime_error{"Unable to read audio file"};
            }

            sf_count_t bytesRead;
            constexpr const std::size_t kBufferSize{1024};
            std::array<char, kBufferSize> buffer = {0};
            do {
                bytesRead = audioFile.readRaw(buffer.data(), kBufferSize);
                if (bytesRead > 0) {
                    co_await channel->send(io::const_buffer(buffer.data(), bytesRead));
                }
            } while (bytesRead == kBufferSize);

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

TEST_F(WitSpeechRecognitionTest, CancelRecognizeSpeech)
{
    io::io_context context{1};

    MockFunction<void(std::exception_ptr, RecognitionResult)> callback;
    EXPECT_CALL(callback,
                Call(Truly(exceptionContainsError(Eq(sys::errc::operation_canceled))),
                     notUnderstoodIntent()));

    auto executor = context.get_executor();
    auto channel = std::make_shared<wit::SpeechRecognition::Channel>(executor, kChannelCapacity);
    auto recognition = factory.speech(executor, channel);

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
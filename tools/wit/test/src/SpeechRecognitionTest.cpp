#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "wit/Config.hpp"
#include "wit/Matchers.hpp"
#include "wit/RecognitionFactory.hpp"
#include "wit/SpeechRecognition.hpp"
#include "wit/Utils.hpp"

#include <jarvisto/SecureContext.hpp>

#include <chrono>
#include <exception>
#include <fstream>

using namespace testing;
using namespace jar;

namespace fs = std::filesystem;

namespace {

std::unique_ptr<char[]>
readFile(const fs::path& filePath, std::size_t& fileSize)
{
    std::fstream stream{filePath, std::ios::in | std::ios::binary};
    fileSize = static_cast<long>(fs::file_size(filePath));
    auto fileData = std::make_unique<char[]>(fileSize);
    stream.read(reinterpret_cast<char*>(fileData.get()), std::streamsize(fileSize));
    return fileData;
}

} // namespace

class SpeechRecognitionTest : public Test {
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

wit::Config SpeechRecognitionTest::config;

TEST_F(SpeechRecognitionTest, RecognizeSpeech)
{
    io::io_context context{1};

    MockFunction<void(std::exception_ptr, RecognitionResult)> callback1;
    EXPECT_CALL(callback1, Call(IsFalse(), understoodIntent("light_control")));

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
    static const fs::path kAudioFile{kAssetAudioPath / "turn-off-the-light.raw"};
    io::co_spawn(
        context.get_executor(),
        [channel]() -> io::awaitable<void> {
            std::size_t fileSize{0};
            auto fileData = readFile(kAudioFile, fileSize);
            auto buffer = io::buffer(fileData.get(), fileSize);
            co_await channel->send(buffer);
            channel->close();
        },
        [&](const std::exception_ptr& eptr) {
            if (eptr) {
                context.stop();
            }
        });

    context.run();
}

TEST_F(SpeechRecognitionTest, CancelRecognizeSpeech)
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
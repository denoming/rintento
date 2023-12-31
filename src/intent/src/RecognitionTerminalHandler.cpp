#include "intent/RecognitionTerminalHandler.hpp"

namespace jar {

std::shared_ptr<RecognitionHandler>
RecognitionTerminalHandler::create(Stream& stream)
{
    // clang-format off
    return std::shared_ptr<RecognitionTerminalHandler>(
        new RecognitionTerminalHandler(stream)
    );
    // clang-format on
}

RecognitionTerminalHandler::RecognitionTerminalHandler(Stream& stream)
    : RecognitionHandler{stream}
{
}

io::awaitable<RecognitionResult>
RecognitionTerminalHandler::handle()
{
    const auto error = sys::errc::make_error_code(sys::errc::operation_not_supported);
    co_await sendResponse(error);
    co_return RecognitionResult{};
}

} // namespace jar

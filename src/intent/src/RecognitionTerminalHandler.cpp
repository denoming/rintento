#include "intent/RecognitionTerminalHandler.hpp"

#include "intent/RecognitionConnection.hpp"

namespace jar {

std::shared_ptr<RecognitionHandler>
RecognitionTerminalHandler::create(std::shared_ptr<RecognitionConnection> connection)
{
    // clang-format off
    return std::shared_ptr<RecognitionTerminalHandler>(
        new RecognitionTerminalHandler(std::move(connection))
    );
    // clang-format on
}

RecognitionTerminalHandler::RecognitionTerminalHandler(
    std::shared_ptr<RecognitionConnection> connection)
    : RecognitionHandler{std::move(connection)}
{
}

void
RecognitionTerminalHandler::handle(Buffer& buffer, Parser& parser)
{
    const auto error = sys::errc::make_error_code(sys::errc::operation_not_supported);
    sendResponse(error);
    submit(error);
}

} // namespace jar

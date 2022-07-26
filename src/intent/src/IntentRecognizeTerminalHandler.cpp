#include "intent/IntentRecognizeTerminalHandler.hpp"

namespace jar {

IntentRecognizeTerminalHandler::IntentRecognizeTerminalHandler(
    IntentRecognizeConnection::Ptr connection, Callback callback)
    : IntentRecognizeHandler{std::move(connection), std::move(callback)}
{
}

void
IntentRecognizeTerminalHandler::handle(Buffer& buffer, Parser& parser)
{
    const auto error = sys::errc::make_error_code(sys::errc::operation_not_supported);
    sendResponse(error);
    submit(error);
}

} // namespace jar

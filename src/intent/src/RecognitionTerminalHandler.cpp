#include "intent/RecognitionTerminalHandler.hpp"

namespace jar {

RecognitionTerminalHandler::RecognitionTerminalHandler(RecognitionConnection::Ptr connection, Callback callback)
    : RecognitionHandler{std::move(connection), std::move(callback)}
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

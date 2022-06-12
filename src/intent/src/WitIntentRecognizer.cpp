#include "intent/WitIntentRecognizer.hpp"

#include "intent/Config.hpp"
#include "intent/WitPendingRecognition.hpp"
#include "intent/WitIntentSession.hpp"

namespace jar {

WitIntentRecognizer::WitIntentRecognizer(net::any_io_executor executor, ssl::context& context)
    : _executor{std::move(executor)}
    , _context{context}
{
}

PendingRecognition::Ptr
WitIntentRecognizer::recognize(std::string_view message, RecognitionCalback callback)
{
    auto session = WitIntentSession::create(_executor, _context);
    session->run(WitBackendHost, WitBackendPort, WitBackendAuth, message, std::move(callback));
    return WitPendingRecognition::create(session);
}

} // namespace jar
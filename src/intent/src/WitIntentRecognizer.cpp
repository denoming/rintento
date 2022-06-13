#include "intent/WitIntentRecognizer.hpp"

#include "intent/Config.hpp"
#include "intent/WitPendingRecognition.hpp"
#include "intent/WitIntentSession.hpp"

namespace jar {

WitIntentRecognizer::WitIntentRecognizer(net::any_io_executor executor)
    : _context{ssl::context::tlsv12_client}
    , _executor{std::move(executor)}
{
    _context.set_default_verify_paths();
    _context.set_verify_mode(ssl::verify_peer);
}

PendingRecognition::Ptr
WitIntentRecognizer::recognize(std::string_view message, RecognitionCalback callback)
{
    auto session = WitIntentSession::create(_context, _executor);
    session->run(WitBackendHost, WitBackendPort, WitBackendAuth, message, std::move(callback));
    return WitPendingRecognition::create(session);
}

} // namespace jar
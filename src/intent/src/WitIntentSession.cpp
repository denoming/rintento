#include "intent/WitIntentSession.hpp"

#include "common/Logger.hpp"

namespace jar {

WitIntentSession::WitIntentSession()
    : _interrupted{false}
{
}

void
WitIntentSession::cancel()
{
    _interrupted = true;
    _cancelSig.emit(net::cancellation_type::terminal);
}

boost::signals2::connection
WitIntentSession::onComplete(const OnCompleteSignal::slot_type& slot)
{
    return _onCompleteSig.connect(slot);
}

boost::signals2::connection
WitIntentSession::onError(const OnErrorSignal::slot_type& slot)
{
    return _onErrorSig.connect(slot);
}

void
WitIntentSession::notifyComplete(const std::string& result)
{
    _onCompleteSig(result);
}

void
WitIntentSession::notifyError(std::error_code error)
{
    _onErrorSig(error);
}

void
WitIntentSession::complete(const std::string& result)
{
    notifyComplete(result);
}

void
WitIntentSession::complete(std::error_code error)
{
    notifyError(error);
}

bool
WitIntentSession::interrupted() const
{
    return _interrupted;
}

net::cancellation_slot
WitIntentSession::onCancel()
{
    return _cancelSig.slot();
}

bool
WitIntentSession::setTlsHostName(beast::ssl_stream<beast::tcp_stream>& stream,
                                 std::string_view hostname,
                                 std::error_code& ec)
{
    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (SSL_set_tlsext_host_name(stream.native_handle(), hostname.data())) {
        ec = {};
    } else {
        ec = sys::error_code{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
    }
    return !ec;
}

void
WitIntentSession::resetTimeout(beast::ssl_stream<beast::tcp_stream>& stream)
{
    beast::get_lowest_layer(stream).expires_after(kHttpTimeout);
}

} // namespace jar
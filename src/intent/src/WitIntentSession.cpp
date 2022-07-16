#include "intent/WitRecognition.hpp"

#include "intent/Constants.hpp"
#include "common/Logger.hpp"

namespace jar {

WitRecognition::WitRecognition()
    : _interrupted{false}
{
}

void
WitRecognition::cancel()
{
    _interrupted = true;

    if (starving()) {
        notifyError(sys::errc::make_error_code(sys::errc::operation_canceled));
    }

    _cancelSig.emit(net::cancellation_type::terminal);
}

boost::signals2::connection
WitRecognition::onComplete(const OnCompleteSignal::slot_type& slot)
{
    return _onCompleteSig.connect(slot);
}

boost::signals2::connection
WitRecognition::onError(const OnErrorSignal::slot_type& slot)
{
    return _onErrorSig.connect(slot);
}

signals::connection
WitRecognition::onData(const OnDataSignal::slot_type& slot)
{
    return _onDataSig.connect(slot);
}

void
WitRecognition::starving(bool value)
{
    _starving = value;
}

bool
WitRecognition::starving() const
{
    return _starving;
}

void
WitRecognition::notifyComplete(const std::string& result)
{
    _onCompleteSig(result);
}

void
WitRecognition::notifyError(std::error_code error)
{
    _onErrorSig(error);
}

void
WitRecognition::notifyData()
{
    _onDataSig();
}

bool
WitRecognition::interrupted() const
{
    return _interrupted;
}

net::cancellation_slot
WitRecognition::onCancel()
{
    return _cancelSig.slot();
}

bool
WitRecognition::setTlsHostName(beast::ssl_stream<beast::tcp_stream>& stream,
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
WitRecognition::resetTimeout(beast::ssl_stream<beast::tcp_stream>& stream)
{
    beast::get_lowest_layer(stream).expires_after(kHttpTimeout);
}

} // namespace jar
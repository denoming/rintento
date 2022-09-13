#include "intent/Http.hpp"

#include "intent/Constants.hpp"

namespace jar {

bool
setTlsHostName(beast::ssl_stream<beast::tcp_stream>& stream,
               std::string_view hostname,
               sys::error_code& ec)
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
resetTimeout(beast::ssl_stream<beast::tcp_stream>& stream)
{
    beast::get_lowest_layer(stream).expires_after(kHttpTimeout);
}

} // namespace jar

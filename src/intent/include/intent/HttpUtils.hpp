#pragma once

#include "Http.hpp"

namespace jar {

bool
setTlsHostName(beast::ssl_stream<beast::tcp_stream>& stream,
               std::string_view hostname,
               sys::error_code& error);

void
resetTimeout(beast::ssl_stream<beast::tcp_stream>& stream);

} // namespace jar
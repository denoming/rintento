#pragma once

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

namespace net = boost::asio;
namespace sys = boost::system;
namespace ssl = net::ssl;
namespace beast = boost::beast;
namespace http = beast::http;

using tcp = net::ip::tcp;

namespace jar {

bool
setTlsHostName(beast::ssl_stream<beast::tcp_stream>& stream,
               std::string_view hostname,
               sys::error_code& error);

void
resetTimeout(beast::ssl_stream<beast::tcp_stream>& stream);

} // namespace jar

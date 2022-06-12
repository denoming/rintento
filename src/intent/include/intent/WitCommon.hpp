#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio.hpp>

namespace net = boost::asio;
namespace sys = boost::system;
namespace ssl = net::ssl;
namespace beast = boost::beast;
namespace http = beast::http;

using tcp = net::ip::tcp;

namespace jar {

static const unsigned int HttpVersion10 = 10;
static const unsigned int HttpVersion11 = 11;

} // namespace jar
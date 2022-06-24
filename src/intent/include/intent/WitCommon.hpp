#pragma once

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include <chrono>

namespace net = boost::asio;
namespace sys = boost::system;
namespace ssl = net::ssl;
namespace beast = boost::beast;
namespace http = beast::http;

using tcp = net::ip::tcp;

namespace jar {

static constexpr auto kHttpTimeout{std::chrono::seconds{15}};

static constexpr std::uint32_t kHttpVersion10 = 10;
static constexpr std::uint32_t kHttpVersion11 = 11;

} // namespace jar
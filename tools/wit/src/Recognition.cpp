#include "wit/Recognition.hpp"

#include <jarvisto/Logger.hpp>

namespace jar::wit {

Recognition::Recognition(io::any_io_executor executor,
                         ssl::context& context,
                         std::string host,
                         std::string port,
                         std::string auth)
    : _stream{std::move(executor), context}
    , _host{std::move(host)}
    , _port{std::move(port)}
    , _auth{std::move(auth)}
{
    BOOST_ASSERT(not _host.empty());
    BOOST_ASSERT(not _port.empty());
    BOOST_ASSERT(not _auth.empty());
}

io::awaitable<Utterances>
Recognition::run()
{
    co_await connect();
    auto result = co_await process();
    co_await shutdown();
    co_return std::move(result);
}

Recognition::Stream&
Recognition::stream()
{
    return _stream;
}

const std::string&
Recognition::host() const
{
    return _host;
}

const std::string&
Recognition::port() const
{
    return _port;
}

const std::string&
Recognition::auth() const
{
    return _auth;
}

io::awaitable<void>
Recognition::connect()
{
    if (_host.empty() || _port.empty() || _auth.empty()) {
        LOGE("Invalid server config options: host<{}>, port<{}>, auth<{}>",
             not _host.empty(),
             not _port.empty(),
             not _auth.empty());
        throw sys::system_error{sys::errc::invalid_argument, sys::generic_category()};
    }

    std::error_code ec;
    net::setServerHostname(_stream, _host, ec);
    if (ec) {
        LOGW("Unable to set server to use in verification process");
    }
    net::setSniHostname(_stream, _host, ec);
    if (ec) {
        LOGW("Unable to set SNI hostname");
    }

    LOGD("Resolve backend address: <{}>", _host);
    tcp::resolver resolver{co_await io::this_coro::executor};
    const auto endpoints = co_await resolver.async_resolve(
        _host, _port, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    if (endpoints.empty()) {
        LOGE("No address has been resolved");
        throw sys::system_error{sys::errc::address_not_available, sys::generic_category()};
    }
    LOGD("Resolving backend address was done: endpoints<{}>", endpoints.size());

    LOGD("Connect to endpoints");
    net::resetTimeout(_stream);
    const auto endpoint = co_await get_lowest_layer(_stream).async_connect(
        endpoints, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Connecting to <{}> endpoint was done", endpoint.address().to_string());

    LOGD("Handshake with host");
    net::resetTimeout(_stream);
    co_await _stream.async_handshake(ssl::stream_base::client,
                                     io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Handshaking was done");
}

io::awaitable<wit::Utterances>
Recognition::process()
{
    co_return wit::Utterances{};
}

io::awaitable<void>
Recognition::shutdown()
{
    net::resetTimeout(_stream);

    LOGD("Shutdown stream");
    std::ignore = co_await _stream.async_shutdown(io::as_tuple(io::use_awaitable));
    LOGD("Shutdown connection was done");
}

} // namespace jar::wit
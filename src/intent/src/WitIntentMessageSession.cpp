#include "intent/WitIntentMessageSession.hpp"

#include "intent/Uri.hpp"
#include "common/Logger.hpp"

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <chrono>

namespace jar {

WitIntentMessageSession::WitIntentMessageSession(ssl::context& context,
                                                 net::any_io_executor& executor)
    : _resolver{executor}
    , _stream{executor, context}
{
}

void
WitIntentMessageSession::run(std::string_view host,
                             std::string_view port,
                             std::string_view auth,
                             std::string_view message)
{
    using namespace std::chrono;

    static constexpr std::string_view kTargetFormat{"/message?v={:%Y%m%d}&q={}"};

    assert(!host.empty());
    assert(!port.empty());
    assert(!auth.empty());
    assert(!message.empty());

    std::error_code ec;
    if (!setTlsHostName(_stream, host, ec)) {
        LOGE("Failed to set TLS hostname: ", ec.message());
        complete(ec);
        return;
    }

    _request.version(kHttpVersion11);
    _request.method(http::verb::get);
    _request.target(fmt::format(kTargetFormat, system_clock::now(), uri::encode(message)));
    _request.set(http::field::host, host);
    _request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    _request.set(http::field::authorization, auth);

    _resolver.async_resolve(
        host,
        port,
        beast::bind_front_handler(&WitIntentMessageSession::onResolveDone, shared_from_this()));
}

void
WitIntentMessageSession::cancel()
{
}

WitIntentMessageSession::Ptr
WitIntentMessageSession::create(ssl::context& context, net::any_io_executor& executor)
{
    return std::shared_ptr<WitIntentMessageSession>(new WitIntentMessageSession(context, executor));
}

void
WitIntentMessageSession::onResolveDone(sys::error_code ec,
                                       const tcp::resolver::results_type& result)
{
    if (ec) {
        LOGE("Failed to resolve address: <{}>", ec.what());
        complete(ec);
        return;
    }

    LOGD("Resolve address was successful: <{}>", result.size());
    resetTimeout(_stream);

    beast::get_lowest_layer(_stream).async_connect(
        result,
        beast::bind_front_handler(&WitIntentMessageSession::onConnectDone, shared_from_this()));
}

void
WitIntentMessageSession::onConnectDone(beast::error_code ec,
                                       const tcp::resolver::results_type::endpoint_type& endpoint)
{
    if (ec) {
        LOGE("Failed to connect: <{}>", ec.what());
        complete(ec);
        return;
    }

    LOGD("Connection with <{}> was established", endpoint.address().to_string());

    _stream.async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(&WitIntentMessageSession::onHandshakeDone, shared_from_this()));
}

void
WitIntentMessageSession::onHandshakeDone(sys::error_code ec)
{
    if (ec) {
        LOGE("Failed to handshake: <{}>", ec.what());
        beast::get_lowest_layer(_stream).close();
        complete(ec);
        return;
    }

    LOGD("Handshaking was successful");
    resetTimeout(_stream);

    http::async_write(
        _stream,
        _request,
        beast::bind_front_handler(&WitIntentMessageSession::onWriteDone, shared_from_this()));
}

void
WitIntentMessageSession::onWriteDone(sys::error_code ec, std::size_t bytesTransferred)
{
    if (ec) {
        LOGE("Failed to write request: <{}>", ec.what());
        beast::get_lowest_layer(_stream).close();
        complete(ec);
        return;
    }

    LOGD("Writing of request was successful: <{}> bytes", bytesTransferred);

    http::async_read(
        _stream,
        _buffer,
        _response,
        beast::bind_front_handler(&WitIntentMessageSession::onReadDone, shared_from_this()));
}

void
WitIntentMessageSession::onReadDone(sys::error_code ec, std::size_t bytesTransferred)
{
    if (ec) {
        LOGE("Failed to read response: <{}>", ec.what());
        beast::get_lowest_layer(_stream).close();
        complete(ec);
        return;
    }

    LOGD("Reading of response was successful: <{}> bytes", bytesTransferred);
    resetTimeout(_stream);

    _stream.async_shutdown(
        beast::bind_front_handler(&WitIntentMessageSession::onShutdownDone, shared_from_this()));
}

void
WitIntentMessageSession::onShutdownDone(sys::error_code ec)
{
    if (ec == net::error::eof) {
        ec = {};
    }

    if (ec) {
        LOGE("Failed to shutdown connection: <{}>", ec.what());
    } else {
        LOGD("Shutdown of connection was successful");
    }

    complete(_response.body());

    beast::get_lowest_layer(_stream).close();
}

} // namespace jar
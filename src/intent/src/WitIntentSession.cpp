#include "intent/WitIntentSession.hpp"

#include "intent/Uri.hpp"

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <iostream>
#include <chrono>

namespace jar {

WitIntentSession::WitIntentSession(ssl::context& context, net::any_io_executor& executor)
    : _resolver{executor}
    , _stream{executor, context}
{
}

void
WitIntentSession::run(std::string_view host,
                      std::string_view port,
                      std::string_view auth,
                      std::string_view message,
                      RecognitionCalback callback)
{
    using namespace std::chrono;

    static const std::string_view kRequestFormat{"/message?v={:%Y%m%d}&q={}"};

    assert(!host.empty());
    assert(!port.empty());
    assert(!auth.empty());
    assert(!message.empty());

    assert(callback);
    _callback = std::move(callback);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(_stream.native_handle(), host.data())) {
        sys::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        std::cerr << ec.message() << "\n";
        return;
    }

    _request.version(HttpVersion11);
    _request.method(http::verb::get);
    _request.target(fmt::format(kRequestFormat, system_clock::now(), uri::encode(message)));
    _request.set(http::field::host, host);
    _request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    _request.set(http::field::authorization, auth);

    _resolver.async_resolve(
        host,
        port,
        beast::bind_front_handler(&WitIntentSession::onResolveDone, shared_from_this()));
}

void
WitIntentSession::cancel()
{
}

WitIntentSession::Ptr
WitIntentSession::create(ssl::context& context, net::any_io_executor& executor)
{
    return std::shared_ptr<WitIntentSession>(new WitIntentSession(context, executor));
}

void
WitIntentSession::onResolveDone(sys::error_code ec, tcp::resolver::results_type result)
{
    if (ec) {
        std::cerr << ec.what() << '\n';
        return;
    }

    beast::get_lowest_layer(_stream).async_connect(
        result, beast::bind_front_handler(&WitIntentSession::onConnectDone, shared_from_this()));
}

void
WitIntentSession::onConnectDone(beast::error_code ec,
                                tcp::resolver::results_type::endpoint_type endpoint)
{
    boost::ignore_unused(endpoint);

    if (ec) {
        std::cerr << ec.what() << '\n';
        return;
    }

    _stream.async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(&WitIntentSession::onHandshakeDone, shared_from_this()));
}

void
WitIntentSession::onHandshakeDone(sys::error_code ec)
{
    if (ec) {
        std::cerr << ec.what() << '\n';
        return;
    }

    http::async_write(
        _stream,
        _request,
        beast::bind_front_handler(&WitIntentSession::onWriteDone, shared_from_this()));
}

void
WitIntentSession::onWriteDone(sys::error_code ec, std::size_t bytesTransferred)
{
    boost::ignore_unused(bytesTransferred);

    if (ec) {
        std::cerr << ec.what() << '\n';
        return;
    }

    http::async_read(_stream,
                     _buffer,
                     _response,
                     beast::bind_front_handler(&WitIntentSession::onReadDone, shared_from_this()));
}

void
WitIntentSession::onReadDone(sys::error_code ec, std::size_t bytesTransferred)
{
    boost::ignore_unused(bytesTransferred);

    if (ec) {
        std::cerr << ec.what() << '\n';
        return;
    }

    _stream.async_shutdown(
        beast::bind_front_handler(&WitIntentSession::onShutdownDone, shared_from_this()));
}

void
WitIntentSession::onShutdownDone(sys::error_code ec)
{
    if (ec == net::error::eof) {
        ec = {};
    }

    if (ec) {
        std::cerr << ec.what() << '\n';
    }

    assert(_callback);
    _callback(_response.body());
}

} // namespace jar
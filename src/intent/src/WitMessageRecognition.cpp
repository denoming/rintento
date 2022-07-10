#include "intent/WitMessageRecognition.hpp"

#include "common/Logger.hpp"
#include "intent/Constants.hpp"
#include "intent/Utils.hpp"
#include "intent/Config.hpp"

namespace jar {

WitMessageRecognition::WitMessageRecognition(ssl::context& context, net::any_io_executor& executor)
    : _resolver{executor}
    , _stream{executor, context}
{
}

void
WitMessageRecognition::run(std::string_view message)
{
    run(WitBackendHost, WitBackendPort, WitBackendAuth, message);
}

void
WitMessageRecognition::run(std::string_view host,
                           std::string_view port,
                           std::string_view auth,
                           std::string_view message)
{
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
    _request.target(format::messageTargetWithDate(message));
    _request.set(http::field::host, host);
    _request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    _request.set(http::field::authorization, auth);
    _request.set(http::field::content_type, "application/json");

    _resolver.async_resolve(
        host,
        port,
        net::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onResolveDone, shared_from_this())));
}

WitMessageRecognition::Ptr
WitMessageRecognition::create(ssl::context& context, net::any_io_executor& executor)
{
    return std::shared_ptr<WitMessageRecognition>(new WitMessageRecognition(context, executor));
}

void
WitMessageRecognition::onResolveDone(sys::error_code error,
                                     const tcp::resolver::results_type& result)
{
    if (error) {
        LOGE("Failed to resolve address: <{}>", error.what());
        complete(error);
        return;
    }

    if (interrupted()) {
        LOGD("Operation was interrupted");
        complete(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    LOGD("Resolve address was successful: <{}>", result.size());
    resetTimeout(_stream);

    beast::get_lowest_layer(_stream).async_connect(
        result,
        net::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onConnectDone, shared_from_this())));
}

void
WitMessageRecognition::onConnectDone(sys::error_code error,
                                     const tcp::resolver::results_type::endpoint_type& endpoint)
{
    if (error) {
        LOGE("Failed to connect: <{}>", error.what());
        complete(error);
        return;
    }

    if (interrupted()) {
        LOGD("Operation was interrupted");
        complete(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    LOGD("Connection with <{}> was established", endpoint.address().to_string());
    resetTimeout(_stream);

    _stream.async_handshake(ssl::stream_base::client,
                            net::bind_cancellation_slot(
                                onCancel(),
                                beast::bind_front_handler(&WitMessageRecognition::onHandshakeDone,
                                                          shared_from_this())));
}

void
WitMessageRecognition::onHandshakeDone(sys::error_code error)
{
    if (error) {
        LOGE("Failed to handshake: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        complete(error);
        return;
    }

    if (interrupted()) {
        LOGD("Operation was interrupted");
        complete(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    LOGD("Handshaking was successful");
    resetTimeout(_stream);

    http::async_write(
        _stream,
        _request,
        net::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onWriteDone, shared_from_this())));
}

void
WitMessageRecognition::onWriteDone(sys::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Failed to write request: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        complete(error);
        return;
    }

    if (interrupted()) {
        LOGD("Operation was interrupted");
        complete(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    LOGD("Writing of request was successful: <{}> bytes", bytesTransferred);
    resetTimeout(_stream);

    http::async_read(
        _stream,
        _buffer,
        _response,
        net::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onReadDone, shared_from_this())));
}

void
WitMessageRecognition::onReadDone(sys::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Failed to read response: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        complete(error);
        return;
    }

    if (interrupted()) {
        LOGD("Operation was interrupted");
        complete(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    LOGD("Reading of response was successful: <{}> bytes", bytesTransferred);
    resetTimeout(_stream);

    _stream.async_shutdown(net::bind_cancellation_slot(
        onCancel(),
        beast::bind_front_handler(&WitMessageRecognition::onShutdownDone, shared_from_this())));
}

void
WitMessageRecognition::onShutdownDone(sys::error_code error)
{
    if (error == net::error::eof || error == sys::errc::operation_canceled) {
        error = {};
    }

    if (error) {
        LOGE("Failed to shutdown connection: <{}>", error.what());
    } else {
        LOGD("Shutdown of connection was successful");
    }

    complete(_response.body());

    beast::get_lowest_layer(_stream).close();
}

} // namespace jar
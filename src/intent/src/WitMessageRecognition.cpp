#include "intent/WitMessageRecognition.hpp"

#include "common/Config.hpp"
#include "intent/Constants.hpp"
#include "intent/Utils.hpp"
#include "intent/WitIntentParser.hpp"
#include "jarvis/Logger.hpp"

namespace jar {

std::shared_ptr<WitMessageRecognition>
WitMessageRecognition::create(std::shared_ptr<Config> config,
                              ssl::context& context,
                              io::any_io_executor executor)
{
    // clang-format off
    return std::shared_ptr<WitMessageRecognition>(
        new WitMessageRecognition(config, context, std::move(executor))
    );
    // clang-format on
}

WitMessageRecognition::WitMessageRecognition(std::shared_ptr<Config> config,
                                             ssl::context& context,
                                             io::any_io_executor executor)
    : _config{std::move(config)}
    , _executor{std::move(executor)}
    , _resolver{_executor}
    , _stream{_executor, context}
{
}

void
WitMessageRecognition::run()
{
    const auto host = _config->recognizeServerHost();
    const auto port = _config->recognizeServerPort();
    const auto auth = _config->recognizeServerAuth();

    if (host.empty() || (port == 0) || auth.empty()) {
        LOGE("Recognize server options are invalid: host<{}>, port<{}>, auth<{}>",
             !host.empty(),
             (port > 0),
             !auth.empty());
        io::post(_executor, [self = shared_from_this()]() {
            self->setError(sys::errc::make_error_code(sys::errc::invalid_argument));
        });
    } else {
        run(host, port, auth);
    }
}

void
WitMessageRecognition::run(std::string_view host, std::uint16_t port, std::string_view auth)
{
    assert(!host.empty());
    assert(port > 0);
    assert(!auth.empty());

    std::error_code error;
    net::setServerHostname(_stream, host, error);
    if (error) {
        LOGW("Unable to set server to use in verification process");
        error = {};
    }
    net::setSniHostname(_stream, host, error);
    if (error) {
        LOGW("Unable to set SNI hostname");
    }

    _req.version(kHttpVersion11);
    _req.method(http::verb::get);
    _req.set(http::field::host, host);
    _req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    _req.set(http::field::authorization, auth);
    _req.set(http::field::content_type, "application/json");

    resolve(host, port);
}

void
WitMessageRecognition::feed(std::string_view message)
{
    if (!starving()) {
        throw std::logic_error{"Inappropriate call to feed-up by data"};
    }

    assert(!message.empty());
    _req.target(format::messageTargetWithDate(message));

    _stream.get_executor().execute([this]() {
        starving(false);
        write();
    });
}

void
WitMessageRecognition::resolve(std::string_view host, std::uint16_t port)
{
    LOGD("Resolve given host address: <{}:{}>", host, port);

    auto portStr = std::to_string(port);
    _resolver.async_resolve(
        host,
        portStr,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onResolveDone, shared_from_this())));
}

void
WitMessageRecognition::onResolveDone(sys::error_code error,
                                     const tcp::resolver::results_type& result)
{
    if (error) {
        LOGE("Failed to resolve address: <{}>", error.what());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    if (result.empty()) {
        LOGE("No address of given host available");
        setError(sys::errc::make_error_code(sys::errc::address_not_available));
    } else {
        LOGD("Resolve address was successful: <{}>", result.size());
        connect(result);
    };
}

void
WitMessageRecognition::connect(const tcp::resolver::results_type& addresses)
{
    LOGD("Connect to host endpoints");

    net::resetTimeout(_stream);

    beast::get_lowest_layer(_stream).async_connect(
        addresses,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onConnectDone, shared_from_this())));
}

void
WitMessageRecognition::onConnectDone(sys::error_code error,
                                     const tcp::resolver::results_type::endpoint_type& endpoint)
{
    if (error) {
        LOGE("Failed to connect: <{}>", error.what());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    } else {
        LOGD("Connecting to host <{}> address was done", endpoint.address().to_string());
        handshake();
    }
}

void
WitMessageRecognition::handshake()
{
    LOGD("Handshake with host");

    net::resetTimeout(_stream);

    _stream.async_handshake(ssl::stream_base::client,
                            io::bind_cancellation_slot(
                                onCancel(),
                                beast::bind_front_handler(&WitMessageRecognition::onHandshakeDone,
                                                          shared_from_this())));
}

void
WitMessageRecognition::onHandshakeDone(sys::error_code error)
{
    if (error) {
        LOGE("Failed to handshake: <{}>", error.what());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    } else {
        LOGD("Handshaking was successful");
        starving(true);
    }
}

void
WitMessageRecognition::write()
{
    LOGD("Write request to stream");

    net::resetTimeout(_stream);

    http::async_write(
        _stream,
        _req,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onWriteDone, shared_from_this())));
}

void
WitMessageRecognition::onWriteDone(sys::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Failed to write request: <{}>", error.what());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    } else {
        LOGD("Writing of request was successful: <{}> bytes", bytesTransferred);
        read();
    }
}

void
WitMessageRecognition::read()
{
    LOGD("Read response from stream");

    net::resetTimeout(_stream);

    http::async_read(
        _stream,
        _buf,
        _res,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onReadDone, shared_from_this())));
}

void
WitMessageRecognition::onReadDone(sys::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Failed to read response: <{}>", error.what());
        setError(error);
        return;
    }

    LOGD("Reading of response was successful: <{}> bytes", bytesTransferred);
    submit(_res.body());

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    } else {
        shutdown();
    }
}

void
WitMessageRecognition::shutdown()
{
    LOGD("Shutdown connection with host");

    net::resetTimeout(_stream);

    _stream.async_shutdown(io::bind_cancellation_slot(
        onCancel(),
        beast::bind_front_handler(&WitMessageRecognition::onShutdownDone, shared_from_this())));
}

void
WitMessageRecognition::onShutdownDone(sys::error_code error)
{
    if (error == io::error::eof || error == sys::errc::operation_canceled) {
        error = {};
    }

    if (error == ssl::error::stream_truncated) {
        LOGD("Stream was truncated");
        error = {};
    }

    if (error) {
        LOGE("Failed to shutdown connection: <{}>", error.what());
    } else {
        LOGD("Shutdown of connection was successful");
    }

    _req.clear();
    _res.clear();
    _buf.clear();
}

} // namespace jar
#include "intent/WitMessageRecognition.hpp"

#include "common/Config.hpp"
#include "intent/Utils.hpp"
#include "intent/WitIntentParser.hpp"
#include "jarvis/Logger.hpp"

#include <boost/assert.hpp>

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

    if (host.empty() || port.empty() || auth.empty()) {
        LOGE("Invalid server config options: host<{}>, port<{}>, auth<{}>",
             !host.empty(),
             !port.empty(),
             !auth.empty());
        setError(sys::errc::make_error_code(sys::errc::invalid_argument));
    } else {
        run(host, port, auth);
    }
}

void
WitMessageRecognition::run(std::string_view host, std::string_view port, std::string_view auth)
{
    BOOST_ASSERT(!host.empty());
    BOOST_ASSERT(!port.empty());
    BOOST_ASSERT(!auth.empty());

    std::error_code error;
    net::setServerHostname(_stream, host, error);
    if (error) {
        LOGW("Unable to set server to use in verification process");
    }
    net::setSniHostname(_stream, host, error);
    if (error) {
        LOGW("Unable to set SNI hostname");
    }

    _req.version(net::kHttpVersion11);
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
    if (!needData()) {
        throw std::logic_error{"Inappropriate call to feed-up by message data"};
    }

    LOGD("Feeding recognition by <{}> bytes message data", message.size());
    needData(false);

    BOOST_ASSERT(!message.empty());
    io::post(_executor,
             [weakSelf = weak_from_this(), target = format::messageTargetWithDate(message)]() {
                 if (auto self = weakSelf.lock()) {
                     self->write(target);
                 }
             });
}

void
WitMessageRecognition::resolve(std::string_view host, std::string_view port)
{
    LOGD("Resolving backend address: <{}>", host);

    _resolver.async_resolve(
        host,
        port,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onResolveDone, shared_from_this())));
}

void
WitMessageRecognition::onResolveDone(std::error_code error,
                                     const tcp::resolver::results_type& result)
{
    if (error) {
        LOGE("Resolving backend address has failed: <{}>", error.message());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(std::make_error_code(std::errc::operation_canceled));
        return;
    }

    if (result.empty()) {
        LOGE("No address has been resolved");
        setError(std::make_error_code(std::errc::address_not_available));
    } else {
        LOGD("The <{}> endpoints was resolved", result.size());
        connect(result);
    };
}

void
WitMessageRecognition::connect(const tcp::resolver::results_type& addresses)
{
    LOGD("Connecting to endpoints: {}", addresses.size());

    net::resetTimeout(_stream);

    beast::get_lowest_layer(_stream).async_connect(
        addresses,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onConnectDone, shared_from_this())));
}

void
WitMessageRecognition::onConnectDone(std::error_code error,
                                     const tcp::resolver::results_type::endpoint_type& endpoint)
{
    if (error) {
        LOGE("Connecting to endpoints has failed: <{}>", error.message());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        LOGD("Connecting to endpoint <{}> endpoint was done", endpoint.address().to_string());
        handshake();
    }
}

void
WitMessageRecognition::handshake()
{
    LOGD("Handshaking with endpoint");

    net::resetTimeout(_stream);

    _stream.async_handshake(ssl::stream_base::client,
                            io::bind_cancellation_slot(
                                onCancel(),
                                beast::bind_front_handler(&WitMessageRecognition::onHandshakeDone,
                                                          shared_from_this())));
}

void
WitMessageRecognition::onHandshakeDone(std::error_code error)
{
    if (error) {
        LOGE("Handshaking with host has failed: <{}>", error.message());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        LOGD("Handshaking has succeeded");
        onCancel().assign([this](io::cancellation_type_t) {
            setError(std::make_error_code(std::errc::operation_canceled));
        });
        needData(true);
    }
}

void
WitMessageRecognition::write(const std::string& target)
{
    LOGD("Writing request to the stream");

    net::resetTimeout(_stream);

    BOOST_ASSERT(!target.empty());
    _req.target(target);

    http::async_write(
        _stream,
        _req,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitMessageRecognition::onWriteDone, shared_from_this())));
}

void
WitMessageRecognition::onWriteDone(std::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Writing request to the stream has failed: <{}>", error.message());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        LOGD("Writing request to the stream has succeeded: <{}> bytes", bytesTransferred);
        read();
    }
}

void
WitMessageRecognition::read()
{
    LOGD("Reading response from the stream");

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
WitMessageRecognition::onReadDone(std::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Reading response from the stream has failed: <{}>", error.message());
        setError(error);
        return;
    }

    LOGD("Reading response from the stream has succeeded: <{}> bytes", bytesTransferred);
    setResult(_res.body());

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(std::make_error_code(std::errc::operation_canceled));
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
WitMessageRecognition::onShutdownDone(std::error_code error)
{
    if (error == sys::error_code{io::error::eof}) {
        error = {};
    }
    if (error == sys::error_code{io::error::operation_aborted}) {
        error = {};
    }
    if (error == sys::error_code(ssl::error::stream_truncated)) {
        error = {};
    }

    if (error) {
        LOGE("Shutdown connection has failed: <{}>", error.message());
    } else {
        LOGD("Shutdown connection has succeeded");
    }

    _req.clear();
    _res.clear();
    _buf.clear();
}

} // namespace jar
#include "intent/WitSpeechRecognition.hpp"

#include "common/Config.hpp"
#include "intent/Constants.hpp"
#include "intent/Utils.hpp"
#include "jarvis/Logger.hpp"

#include <boost/assert.hpp>

namespace jar {

std::shared_ptr<WitSpeechRecognition>
WitSpeechRecognition::create(std::shared_ptr<Config> config,
                             ssl::context& context,
                             io::any_io_executor executor)
{
    // clang-format off
    return std::shared_ptr<WitSpeechRecognition>(
        new WitSpeechRecognition(std::move(config), context, executor)
    );
    // clang-format on
}

WitSpeechRecognition::WitSpeechRecognition(std::shared_ptr<Config> config,
                                           ssl::context& context,
                                           io::any_io_executor executor)
    : _config{std::move(config)}
    , _executor{std::move(executor)}
    , _resolver{_executor}
    , _stream{_executor, context}
{
}

void
WitSpeechRecognition::run()
{
    BOOST_ASSERT(_config);

    const auto host = _config->recognizeServerHost();
    const auto port = _config->recognizeServerPort();
    const auto auth = _config->recognizeServerAuth();

    if (host.empty() || (port.empty()) || auth.empty()) {
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
WitSpeechRecognition::run(std::string_view host, std::string_view port, std::string_view auth)
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

    _req.version(kHttpVersion11);
    _req.target(format::speechTargetWithDate());
    _req.method(http::verb::post);
    _req.set(http::field::host, host);
    _req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    _req.set(http::field::authorization, auth);
    _req.set(http::field::content_type,
             "audio/raw; encoding=signed-integer; bits=16; rate=16000; endian=little");
    _req.set(http::field::transfer_encoding, "chunked");
    _req.set(http::field::expect, "100-continue");

    resolve(host, port);
}

void
WitSpeechRecognition::feed(io::const_buffer buffer)
{
    if (!starving()) {
        throw std::logic_error{"Inappropriate call to feed-up by data"};
    }

    BOOST_ASSERT(buffer.size() > 0);
    starving(false);
    io::dispatch(_executor, [weakSelf = weak_from_this(), buffer]() {
        if (auto self = weakSelf.lock()) {
            self->writeNextChunk(buffer);
        }
    });
}

void
WitSpeechRecognition::finalize()
{
    if (!starving()) {
        throw std::logic_error{"Inappropriate call to feed-up by data"};
    }

    starving(false);

    io::dispatch(_executor, [weakSelf = weak_from_this()]() {
        if (auto self = weakSelf.lock()) {
            self->writeLastChunk();
        }
    });
}

void
WitSpeechRecognition::resolve(std::string_view host, std::string_view port)
{
    LOGD("Resolve backend address: <{}>", host);

    _resolver.async_resolve(
        host,
        port,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitSpeechRecognition::onResolveDone, shared_from_this())));
}

void
WitSpeechRecognition::onResolveDone(sys::error_code error,
                                    const tcp::resolver::results_type& result)
{
    if (error) {
        LOGE("Failed to resolve: <{}>", error.what());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    } else {
        LOGD("Resolve address was successful: <{}>", result.size());
        connect(result);
    }
}

void
WitSpeechRecognition::connect(const tcp::resolver::results_type& addresses)
{
    LOGD("Connect to host endpoints");

    net::resetTimeout(_stream);

    beast::get_lowest_layer(_stream).async_connect(
        addresses,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitSpeechRecognition::onConnectDone, shared_from_this())));
}

void
WitSpeechRecognition::onConnectDone(sys::error_code error,
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
WitSpeechRecognition::handshake()
{
    LOGD("Handshake with host");

    net::resetTimeout(_stream);

    _stream.async_handshake(
        ssl::stream_base::client,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitSpeechRecognition::onHandshakeDone, shared_from_this())));
}

void
WitSpeechRecognition::onHandshakeDone(sys::error_code error)
{
    if (error) {
        LOGE("Failed to handshake: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    } else {
        LOGD("Handshaking was successful");
        readContinue();
    }
}

void
WitSpeechRecognition::readContinue()
{
    net::resetTimeout(_stream);

    sys::error_code error;
    http::request_serializer<http::empty_body, http::fields> hs{_req};
    const auto bytesTransferred = http::write_header(_stream, hs, error);
    if (error) {
        LOGE("Failed to write request header: <{}>", error.what());
        setError(error);
    } else {
        LOGD("Writing of request header was successful: <{}> bytes", bytesTransferred);
        http::async_read(_stream,
                         _buf,
                         _res,
                         io::bind_cancellation_slot(
                             onCancel(),
                             beast::bind_front_handler(&WitSpeechRecognition::onReadContinueDone,
                                                       shared_from_this())));
    }
}

void
WitSpeechRecognition::onReadContinueDone(sys::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Failed to read continue: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        setError(error);
        return;
    }

    if (_res.result() != http::status::continue_) {
        LOGE("Continue reading is not available");
        beast::get_lowest_layer(_stream).close();
        setError(std::make_error_code(std::errc::illegal_byte_sequence));
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    } else {
        LOGD("Ready to provide speech data");
        starving(true);
    }
}

void
WitSpeechRecognition::writeNextChunk(io::const_buffer buffer)
{
    net::resetTimeout(_stream);

    io::async_write(_stream,
                    http::make_chunk(buffer),
                    io::bind_cancellation_slot(
                        onCancel(),
                        beast::bind_front_handler(&WitSpeechRecognition::onWriteNextChunkDone,
                                                  shared_from_this())));
}

void
WitSpeechRecognition::onWriteNextChunkDone(sys::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Failed to write next chunk: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    } else {
        LOGD("Writing of next chunk was successful: {} bytes", bytesTransferred);
        starving(true);
    }
}

void
WitSpeechRecognition::writeLastChunk()
{
    net::resetTimeout(_stream);

    io::async_write(_stream,
                    http::make_chunk_last(),
                    io::bind_cancellation_slot(
                        onCancel(),
                        beast::bind_front_handler(&WitSpeechRecognition::onWriteLastChunkDone,
                                                  shared_from_this())));
}

void
WitSpeechRecognition::onWriteLastChunkDone(sys::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Failed to write last chunk: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    } else {
        LOGD("Writing of last chunk was successful: {} bytes", bytesTransferred);
        read();
    }
}

void
WitSpeechRecognition::read()
{
    net::resetTimeout(_stream);

    http::async_read(
        _stream,
        _buf,
        _res,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitSpeechRecognition::onReadDone, shared_from_this())));
}

void
WitSpeechRecognition::onReadDone(sys::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Failed to read: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        setError(error);
        return;
    }

    LOGD("Reading was successful: <{}> bytes", bytesTransferred);
    submit(_res.body());

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
    } else {
        shutdown();
    }
}

void
WitSpeechRecognition::shutdown()
{
    LOGD("Shutdown connection with host");

    net::resetTimeout(_stream);

    _stream.async_shutdown(io::bind_cancellation_slot(
        onCancel(),
        beast::bind_front_handler(&WitSpeechRecognition::onShutdownDone, shared_from_this())));
}

void
WitSpeechRecognition::onShutdownDone(sys::error_code error)
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
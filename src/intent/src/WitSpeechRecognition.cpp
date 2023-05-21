#include "intent/WitSpeechRecognition.hpp"

#include "common/Config.hpp"
#include "intent/Utils.hpp"
#include "intent/WitIntentParser.hpp"

#include <jarvis/Logger.hpp>

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

    _req.version(net::kHttpVersion11);
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
    if (!needData()) {
        throw std::logic_error{"Inappropriate call to feed-up by speech data"};
    }

    LOGD("Feeding by <{}> bytes of speech data", buffer.size());
    needData(false);

    BOOST_ASSERT(buffer.size() > 0);
    io::post(_executor, [weakSelf = weak_from_this(), buffer]() {
        if (auto self = weakSelf.lock()) {
            self->writeNextChunk(buffer);
        }
    });
}

void
WitSpeechRecognition::finalize()
{
    if (!needData()) {
        throw std::logic_error{"Inappropriate call to feed-up by speech data"};
    }

    LOGD("Finalizing feeding by speech data");
    needData(false);

    io::post(_executor, [weakSelf = weak_from_this()]() {
        if (auto self = weakSelf.lock()) {
            self->writeLastChunk();
        }
    });
}

void
WitSpeechRecognition::resolve(std::string_view host, std::string_view port)
{
    LOGD("Resolving backend address: <{}>", host);

    _resolver.async_resolve(
        host,
        port,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitSpeechRecognition::onResolveDone, shared_from_this())));
}

void
WitSpeechRecognition::onResolveDone(std::error_code error,
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
    }
}

void
WitSpeechRecognition::connect(const tcp::resolver::results_type& addresses)
{
    LOGD("Connecting to endpoints: {}", addresses.size());

    net::resetTimeout(_stream);

    beast::get_lowest_layer(_stream).async_connect(
        addresses,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitSpeechRecognition::onConnectDone, shared_from_this())));
}

void
WitSpeechRecognition::onConnectDone(std::error_code error,
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
WitSpeechRecognition::handshake()
{
    LOGD("Handshaking with host");

    net::resetTimeout(_stream);

    _stream.async_handshake(
        ssl::stream_base::client,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitSpeechRecognition::onHandshakeDone, shared_from_this())));
}

void
WitSpeechRecognition::onHandshakeDone(std::error_code error)
{
    if (error) {
        LOGE("Handshaking with endpoint has failed: <{}>", error.message());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        LOGD("Handshaking has succeeded");
        readContinue();
    }
}

void
WitSpeechRecognition::readContinue()
{
    net::resetTimeout(_stream);

    LOGD("Writing request header");
    sys::error_code error;
    http::request_serializer<http::empty_body, http::fields> hs{_req};
    http::write_header(_stream, hs, error);
    if (error) {
        LOGE("Writing request header has failed: <{}>", error.what());
        setError(error);
        return;
    }

    LOGD("Reading response");
    http::async_read(_stream,
                     _buf,
                     _res,
                     io::bind_cancellation_slot(
                         onCancel(),
                         beast::bind_front_handler(&WitSpeechRecognition::onReadContinueDone,
                                                   shared_from_this())));
}

void
WitSpeechRecognition::onReadContinueDone(std::error_code error, std::size_t /*bytesTransferred*/)
{
    if (error) {
        LOGE("Reading response has failed: <{}>", error.message());
        setError(error);
        return;
    }

    if (_res.result() != http::status::continue_) {
        LOGE("Invalid continue status in response");
        setError(std::make_error_code(std::errc::illegal_byte_sequence));
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        LOGD("Reading response has succeeded");
        onCancel().assign([this](io::cancellation_type_t) {
            setError(sys::errc::make_error_code(sys::errc::operation_canceled));
        });
        needData(true);
    }
}

void
WitSpeechRecognition::writeNextChunk(io::const_buffer buffer)
{
    net::resetTimeout(_stream);

    LOGD("Writing next chunk with <{}> size", buffer.size());
    io::async_write(_stream,
                    http::make_chunk(buffer),
                    io::bind_cancellation_slot(
                        onCancel(),
                        beast::bind_front_handler(&WitSpeechRecognition::onWriteNextChunkDone,
                                                  shared_from_this())));
}

void
WitSpeechRecognition::onWriteNextChunkDone(std::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Writing next chunk has failed: <{}>", error.message());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        LOGD("Writing next chunk has succeeded: {} bytes", bytesTransferred);
        needData(true);
    }
}

void
WitSpeechRecognition::writeLastChunk()
{
    net::resetTimeout(_stream);

    LOGD("Writing last chunk");
    io::async_write(_stream,
                    http::make_chunk_last(),
                    io::bind_cancellation_slot(
                        onCancel(),
                        beast::bind_front_handler(&WitSpeechRecognition::onWriteLastChunkDone,
                                                  shared_from_this())));
}

void
WitSpeechRecognition::onWriteLastChunkDone(std::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Writing last chunk has failed: <{}>", error.message());
        setError(error);
        return;
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(std::make_error_code(std::errc::operation_canceled));
    } else {
        LOGD("Writing last chunk has succeeded: {} bytes", bytesTransferred);
        read();
    }
}

void
WitSpeechRecognition::read()
{
    net::resetTimeout(_stream);

    LOGD("Reading recognition result");
    http::async_read(
        _stream,
        _buf,
        _res,
        io::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitSpeechRecognition::onReadDone, shared_from_this())));
}

void
WitSpeechRecognition::onReadDone(std::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Reading recognition result has failed: <{}>", error.message());
        beast::get_lowest_layer(_stream).close();
        setError(error);
        return;
    }

    LOGD("Reading recognition result has succeeded: <{}> bytes", bytesTransferred);

    WitIntentParser parser;
    if (auto result = parser.parseSpeechResult(_res.body()); result) {
        setResult(std::move(result.value()));
    } else {
        setError(result.error());
    }

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(std::make_error_code(std::errc::operation_canceled));
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
WitSpeechRecognition::onShutdownDone(std::error_code error)
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
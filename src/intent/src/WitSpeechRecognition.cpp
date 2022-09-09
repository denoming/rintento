#include "intent/WitSpeechRecognition.hpp"

#include "common/Logger.hpp"
#include "intent/Config.hpp"
#include "intent/Constants.hpp"
#include "intent/HttpUtils.hpp"
#include "intent/Utils.hpp"
#include "intent/WitIntentParser.hpp"

#include <fstream>

namespace jar {

WitSpeechRecognition::Ptr
WitSpeechRecognition::create(ssl::context& context, net::any_io_executor executor)
{
    // clang-format off
    return std::shared_ptr<WitSpeechRecognition>(
        new WitSpeechRecognition(context, executor)
    );
    // clang-format on
}

WitSpeechRecognition::WitSpeechRecognition(ssl::context& context, net::any_io_executor executor)
    : _executor{std::move(executor)}
    , _resolver{_executor}
    , _stream{_executor, context}
{
}

void
WitSpeechRecognition::run()
{
    run(WitBackendHost, WitBackendPort, WitBackendAuth);
}

void
WitSpeechRecognition::run(std::string_view host, std::string_view port, std::string_view auth)
{
    assert(!host.empty());
    assert(!port.empty());
    assert(!auth.empty());

    sys::error_code error;
    if (!setTlsHostName(_stream, host, error)) {
        LOGE("Failed to set TLS hostname: ", error.message());
        setError(error);
        return;
    }

    _request.version(kHttpVersion11);
    _request.target(format::speechTargetWithDate());
    _request.method(http::verb::post);
    _request.set(http::field::host, host);
    _request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    _request.set(http::field::authorization, auth);
    _request.set(http::field::content_type,
                 "audio/raw; encoding=signed-integer; bits=16; rate=16000; endian=little");
    _request.set(http::field::transfer_encoding, "chunked");
    _request.set(http::field::expect, "100-continue");

    resolve(host, port);
}

void
WitSpeechRecognition::feed(net::const_buffer buffer)
{
    if (!starving()) {
        throw std::logic_error{"Inappropriate call to feed-up by data"};
    }

    assert(buffer.size() > 0);

    _stream.get_executor().execute([this, buffer]() {
        starving(false);
        writeNextChunk(buffer);
    });
}

void
WitSpeechRecognition::finalize()
{
    if (!starving()) {
        throw std::logic_error{"Inappropriate call to feed-up by data"};
    }

    _stream.get_executor().execute([this]() {
        starving(false);
        writeLastChunk();
    });
}

void
WitSpeechRecognition::resolve(std::string_view host, std::string_view port)
{
    LOGD("Resolve given host address: <{}:{}>", host, port);

    _resolver.async_resolve(
        host,
        port,
        net::bind_cancellation_slot(
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
    resetTimeout(_stream);

    beast::get_lowest_layer(_stream).async_connect(
        addresses,
        net::bind_cancellation_slot(
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

    resetTimeout(_stream);

    _stream.async_handshake(
        ssl::stream_base::client,
        net::bind_cancellation_slot(
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
    resetTimeout(_stream);

    sys::error_code error;
    http::request_serializer<http::empty_body, http::fields> hs{_request};
    const auto bytesTransferred = http::write_header(_stream, hs, error);
    if (error) {
        LOGE("Failed to write request header: <{}>", error.what());
        setError(error);
    } else {
        LOGD("Writing of request header was successful: <{}> bytes", bytesTransferred);
        http::async_read(_stream,
                         _buffer,
                         _response,
                         net::bind_cancellation_slot(
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

    if (_response.result() != http::status::continue_) {
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
WitSpeechRecognition::writeNextChunk(net::const_buffer buffer)
{
    resetTimeout(_stream);

    net::async_write(_stream,
                     http::make_chunk(buffer),
                     net::bind_cancellation_slot(
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
        return;
    }

    LOGD("Writing of next chunk was successful: {} bytes", bytesTransferred);

    starving(true);
}

void
WitSpeechRecognition::writeLastChunk()
{
    resetTimeout(_stream);

    net::async_write(_stream,
                     http::make_chunk_last(),
                     net::bind_cancellation_slot(
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
        return;
    }

    LOGD("Writing of last chunk was successful: {} bytes", bytesTransferred);

    read();
}

void
WitSpeechRecognition::read()
{
    resetTimeout(_stream);

    http::async_read(
        _stream,
        _buffer,
        _response,
        net::bind_cancellation_slot(
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

    if (cancelled()) {
        LOGD("Operation was interrupted");
        setError(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    LOGD("Reading was successful: <{}> bytes", bytesTransferred);

    shutdown();
}

void
WitSpeechRecognition::shutdown()
{
    resetTimeout(_stream);

    _stream.async_shutdown(net::bind_cancellation_slot(
        onCancel(),
        beast::bind_front_handler(&WitSpeechRecognition::onShutdownDone, shared_from_this())));
}

void
WitSpeechRecognition::onShutdownDone(sys::error_code error)
{
    if (error == net::error::eof || error == sys::errc::operation_canceled) {
        error = {};
    }

    if (error) {
        LOGE("Failed to shutdown connection: <{}>", error.what());
    } else {
        LOGD("Shutdown of connection was successful");
    }

    submit(_response.body());

    beast::get_lowest_layer(_stream).close();
}

} // namespace jar
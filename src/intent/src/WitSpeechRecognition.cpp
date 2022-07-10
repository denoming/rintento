#include "intent/WitSpeechRecognition.hpp"

#include "intent/Constants.hpp"
#include "intent/Utils.hpp"
#include "common/Logger.hpp"

#include <fstream>

namespace jar {

WitSpeechRecognition::WitSpeechRecognition(ssl::context& context, net::any_io_executor& executor)
    : _resolver{executor}
    , _stream{executor, context}
{
}

void
WitSpeechRecognition::run(std::string_view host,
                          std::string_view port,
                          std::string_view auth,
                          fs::path data)
{
    assert(!host.empty());
    assert(!port.empty());
    assert(!auth.empty());
    assert(!data.empty());

    std::error_code error;
    if (!setTlsHostName(_stream, host, error)) {
        LOGE("Failed to set TLS hostname: ", error.message());
        complete(error);
        return;
    }

    std::fstream fs{data, std::ios::in | std::ios::binary};
    if (!fs.is_open()) {
        LOGE("Failed to open");
        return;
    }
    _fileSize = static_cast<long>(fs::file_size(data));
    assert(_fileSize > 0);
    _fileData = std::make_unique<std::int8_t[]>(_fileSize);
    fs.read(reinterpret_cast<char*>(_fileData.get()), _fileSize);

    _request.version(kHttpVersion11);
    _request.target();
    _request.method(http::verb::post);
    _request.set(http::field::host, host);
    _request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    _request.set(http::field::authorization, auth);
    _request.set(http::field::content_type,
                 "audio/raw; encoding=signed-integer; bits=16; rate=16000; endian=little");
    _request.set(http::field::transfer_encoding, "chunked");
    _request.set(http::field::expect, "100-continue");

    _resolver.async_resolve(
        host,
        port,
        net::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitSpeechRecognition::onResolveDone, shared_from_this())));
}

WitSpeechRecognition::Ptr
WitSpeechRecognition::create(ssl::context& context, net::any_io_executor& executor)
{
    return std::shared_ptr<WitSpeechRecognition>(new WitSpeechRecognition(context, executor));
}

void
WitSpeechRecognition::onResolveDone(sys::error_code error,
                                    const tcp::resolver::results_type& result)
{
    if (error) {
        LOGE("Failed to resolve: <{}>", error.what());
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
            beast::bind_front_handler(&WitSpeechRecognition::onConnectDone, shared_from_this())));
}

void
WitSpeechRecognition::onConnectDone(sys::error_code error,
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

    http::request_serializer<http::empty_body, http::fields> hs{_request};
    const auto bytesTransferred = http::write_header(_stream, hs, error);
    if (error) {
        LOGE("Failed to write request header: <{}>", error.what());
        complete(error);
        return;
    } else {
        LOGD("Writing of request header was successful: <{}> bytes", bytesTransferred);
    }

    http::async_read(_stream,
                     _buffer,
                     _response,
                     net::bind_cancellation_slot(
                         onCancel(),
                         beast::bind_front_handler(&WitSpeechRecognition::onReadContinueDone,
                                                   shared_from_this())));
}

void
WitSpeechRecognition::onReadContinueDone(sys::error_code error, std::size_t bytesTransferred)
{
    static const int ChunkSize{20000};

    if (error) {
        LOGE("Failed to read continue: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        complete(error);
        return;
    }

    if (_response.result() != http::status::continue_) {
        LOGE("Continue reading is not available");
        beast::get_lowest_layer(_stream).close();
        complete(std::make_error_code(std::errc::illegal_byte_sequence));
        return;
    }

    if (interrupted()) {
        LOGD("Operation was interrupted");
        complete(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    LOGD("Reading of request header was successful: <{}> bytes", bytesTransferred);
    resetTimeout(_stream);

    auto chunk = http::make_chunk(net::const_buffer(_fileData.get(), ChunkSize));
    _fileOffset += ChunkSize, _fileSize -= ChunkSize;

    net::async_write(
        _stream,
        chunk,
        net::bind_cancellation_slot(
            onCancel(),
            beast::bind_front_handler(&WitSpeechRecognition::onWriteDone, shared_from_this())));
}

void
WitSpeechRecognition::onWriteDone(sys::error_code error, std::size_t bytesTransferred)
{
    static const int ChunkSize{20000};

    if (error) {
        LOGE("Failed to write chunk: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        complete(error);
        return;
    }

    if (interrupted()) {
        LOGD("Operation was interrupted");
        complete(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    LOGD("Writing of chunk was successful: {} bytes", bytesTransferred);
    resetTimeout(_stream);

    if (_fileSize > 0) {
        const auto size = std::min<long>(ChunkSize, _fileSize);
        auto chunk = http::make_chunk(net::const_buffer(_fileData.get() + _fileOffset, size));
        _fileSize -= size, _fileOffset += size;
        net::async_write(
            _stream,
            chunk,
            net::bind_cancellation_slot(
                onCancel(),
                beast::bind_front_handler(&WitSpeechRecognition::onWriteDone, shared_from_this())));
    } else {
        net::async_write(
            _stream,
            http::make_chunk_last(),
            net::bind_cancellation_slot(
                onCancel(),
                beast::bind_front_handler(&WitSpeechRecognition::onReadReady, shared_from_this())));
    }
}

void
WitSpeechRecognition::onReadReady(sys::error_code error, std::size_t bytesTransferred)
{
    if (error) {
        LOGE("Failed to write last chunk: <{}>", error.what());
        beast::get_lowest_layer(_stream).close();
        complete(error);
        return;
    }

    if (interrupted()) {
        LOGD("Operation was interrupted");
        complete(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    LOGD("Writing of last chunk was successful: {} bytes", bytesTransferred);
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
        complete(error);
        return;
    }

    if (interrupted()) {
        LOGD("Operation was interrupted");
        complete(sys::errc::make_error_code(sys::errc::operation_canceled));
        return;
    }

    LOGD("Reading was successful: <{}> bytes", bytesTransferred);
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

    complete(_response.body());

    beast::get_lowest_layer(_stream).close();
}

} // namespace jar
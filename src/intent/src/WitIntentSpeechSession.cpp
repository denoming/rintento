#include "intent/WitIntentSpeechSession.hpp"

#include "intent/Uri.hpp"
#include "common/Logger.hpp"

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <chrono>
#include <fstream>

namespace jar {

WitIntentSpeechSession::WitIntentSpeechSession(ssl::context& context,
                                               net::any_io_executor& executor)
    : _resolver{executor}
    , _stream{executor, context}
{
}

void
WitIntentSpeechSession::run(std::string_view host,
                            std::string_view port,
                            std::string_view auth,
                            fs::path data)
{
    using namespace std::chrono;

    static constexpr std::string_view kTargetFormat{"/speech?v={:%Y%m%d}"};

    assert(!host.empty());
    assert(!port.empty());
    assert(!auth.empty());
    assert(!data.empty());

    std::error_code ec;
    if (!setTlsHostName(_stream, host, ec)) {
        LOGE("Failed to set TLS hostname: ", ec.message());
        complete(ec);
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
    _request.target(fmt::format(kTargetFormat, system_clock::now()));
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
        beast::bind_front_handler(&WitIntentSpeechSession::onResolveDone, shared_from_this()));
}

void
WitIntentSpeechSession::cancel()
{
}

WitIntentSpeechSession::Ptr
WitIntentSpeechSession::create(ssl::context& context, net::any_io_executor& executor)
{
    return std::shared_ptr<WitIntentSpeechSession>(new WitIntentSpeechSession(context, executor));
}

void
WitIntentSpeechSession::onResolveDone(sys::error_code ec, const tcp::resolver::results_type& result)
{
    if (ec) {
        LOGE("Failed to resolve: <{}>", ec.what());
        complete(ec);
        return;
    }

    LOGD("Resolve address was successful: <{}>", result.size());
    resetTimeout(_stream);

    beast::get_lowest_layer(_stream).async_connect(
        result,
        beast::bind_front_handler(&WitIntentSpeechSession::onConnectDone, shared_from_this()));
}

void
WitIntentSpeechSession::onConnectDone(sys::error_code ec,
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
        beast::bind_front_handler(&WitIntentSpeechSession::onHandshakeDone, shared_from_this()));
}

void
WitIntentSpeechSession::onHandshakeDone(sys::error_code ec)
{
    if (ec) {
        LOGE("Failed to handshake: <{}>", ec.what());
        beast::get_lowest_layer(_stream).close();
        complete(ec);
        return;
    }

    LOGD("Handshaking was successful");
    resetTimeout(_stream);

    http::request_serializer<http::empty_body, http::fields> hs{_request};
    const auto bytesTransferred = http::write_header(_stream, hs, ec);
    if (ec) {
        LOGE("Failed to write request header: <{}>", ec.what());
        complete(ec);
        return;
    } else {
        LOGD("Writing of request header was successful: <{}> bytes", bytesTransferred);
    }

    http::async_read(
        _stream,
        _buffer,
        _response,
        beast::bind_front_handler(&WitIntentSpeechSession::onReadContinueDone, shared_from_this()));
}

void
WitIntentSpeechSession::onReadContinueDone(sys::error_code ec, std::size_t bytesTransferred)
{
    static const int ChunkSize{20000};

    if (ec) {
        LOGE("Failed to read continue: <{}>", ec.what());
        beast::get_lowest_layer(_stream).close();
        complete(ec);
        return;
    }

    if (_response.result() != http::status::continue_) {
        LOGE("Continue reading is not available");
        beast::get_lowest_layer(_stream).close();
        complete(std::make_error_code(std::errc::illegal_byte_sequence));
        return;
    }

    LOGD("Reading of request header was successful: <{}> bytes", bytesTransferred);
    resetTimeout(_stream);

    auto chunk = http::make_chunk(net::const_buffer(_fileData.get(), ChunkSize));
    _fileOffset += ChunkSize, _fileSize -= ChunkSize;

    net::async_write(
        _stream,
        chunk,
        beast::bind_front_handler(&WitIntentSpeechSession::onWriteDone, shared_from_this()));
}

void
WitIntentSpeechSession::onWriteDone(sys::error_code ec, std::size_t bytesTransferred)
{
    static const int ChunkSize{20000};

    if (ec) {
        LOGE("Failed to write chunk: <{}>", ec.what());
        beast::get_lowest_layer(_stream).close();
        complete(ec);
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
            beast::bind_front_handler(&WitIntentSpeechSession::onWriteDone, shared_from_this()));
    } else {
        net::async_write(
            _stream,
            http::make_chunk_last(),
            beast::bind_front_handler(&WitIntentSpeechSession::onReadReady, shared_from_this()));
    }
}

void
WitIntentSpeechSession::onReadReady(sys::error_code ec, std::size_t bytesTransferred)
{
    if (ec) {
        LOGE("Failed to write last chunk: <{}>", ec.what());
        beast::get_lowest_layer(_stream).close();
        complete(ec);
        return;
    }

    LOGD("Writing of last chunk was successful: {} bytes", bytesTransferred);
    resetTimeout(_stream);

    http::async_read(
        _stream,
        _buffer,
        _response,
        beast::bind_front_handler(&WitIntentSpeechSession::onReadDone, shared_from_this()));
}

void
WitIntentSpeechSession::onReadDone(sys::error_code ec, std::size_t bytesTransferred)
{
    if (ec) {
        LOGE("Failed to read: <{}>", ec.what());
        beast::get_lowest_layer(_stream).close();
        complete(ec);
        return;
    }

    LOGD("Reading was successful: <{}> bytes", bytesTransferred);
    resetTimeout(_stream);

    _stream.async_shutdown(
        beast::bind_front_handler(&WitIntentSpeechSession::onShutdownDone, shared_from_this()));
}

void
WitIntentSpeechSession::onShutdownDone(sys::error_code ec)
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
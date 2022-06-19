#include "intent/WitIntentSpeechSession.hpp"

#include "intent/Uri.hpp"
#include "intent/WitIntentParser.hpp"
#include "common/Logger.hpp"

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <iomanip>

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
                            fs::path data,
                            RecognitionCalback callback)
{
    using namespace std::chrono;

    static constexpr std::string_view kTargetFormat{"/speech?v={:%Y%m%d}"};

    assert(!host.empty());
    assert(!port.empty());
    assert(!auth.empty());
    assert(!data.empty());

    assert(callback);
    _callback = std::move(callback);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(_stream.native_handle(), host.data())) {
        sys::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        LOGE("Failed to set TLS hostname: ", ec.message());
        return;
    }

    LOGI("Current working dir: {}", fs::current_path().string());

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
        return;
    }

    LOGD("Resolve address was successful: <{}>", result.size());

    beast::get_lowest_layer(_stream).expires_after(kHttpTimeout);

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
        return;
    }

    LOGD("Handshaking was successful");

    http::request_serializer<http::empty_body, http::fields> hs{_request};
    http::write_header(_stream, hs);

    beast::get_lowest_layer(_stream).expires_after(kHttpTimeout);
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
        return;
    }

    if (_response.result() != http::status::continue_) {
        LOGE("Continue reading has failed");
        _stream.async_shutdown(
            beast::bind_front_handler(&WitIntentSpeechSession::onShutdownDone, shared_from_this()));
        return;
    }

    auto chunk = http::make_chunk(net::const_buffer(_fileData.get(), ChunkSize));
    _fileOffset += ChunkSize, _fileSize -= ChunkSize;

    beast::get_lowest_layer(_stream).expires_after(kHttpTimeout);
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
        LOGE("Failed to write: <{}>", ec.what());
        return;
    }

    LOGD("Write was successful: {} bytes", bytesTransferred);

    beast::get_lowest_layer(_stream).expires_after(kHttpTimeout);

    if (_fileSize > 0) {
        auto size = std::min<long>(ChunkSize, _fileSize);
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
        return;
    }

    LOGD("Write the last chunk was successful: {} bytes", bytesTransferred);

    beast::get_lowest_layer(_stream).expires_after(kHttpTimeout);

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
        return;
    }

    LOGD("Reading was successful: <{}> bytes", bytesTransferred);

    beast::get_lowest_layer(_stream).expires_after(kHttpTimeout);

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
        LOGE("Failed to shutdown: <{}>", ec.what());
    } else {
        LOGD("Shutdown was successful");
    }

    assert(_callback);
    _callback(_response.body());
}

} // namespace jar
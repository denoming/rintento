#pragma once

#include "intent/WitCommon.hpp"
#include "intent/Types.hpp"

#include <boost/asio/random_access_file.hpp>
#include <boost/beast/http/chunk_encode.hpp>

#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

namespace jar {

class WitIntentSpeechSession : public std::enable_shared_from_this<WitIntentSpeechSession> {
public:
    using Ptr = std::shared_ptr<WitIntentSpeechSession>;

    void
    run(std::string_view host,
        std::string_view port,
        std::string_view auth,
        fs::path data,
        RecognitionCalback callback);

    void
    cancel();

private:
    friend class WitIntentRecognizer;
    explicit WitIntentSpeechSession(ssl::context& context, net::any_io_executor& executor);

    static Ptr
    create(ssl::context& context, net::any_io_executor& executor);

    void
    onResolveDone(sys::error_code ec, const tcp::resolver::results_type& result);

    void
    onConnectDone(sys::error_code ec, const tcp::resolver::results_type::endpoint_type& endpoint);

    void
    onHandshakeDone(sys::error_code ec);

    void
    onReadContinueDone(sys::error_code ec,  std::size_t bytesTransferred);

    void
    onWriteDone(sys::error_code ec, std::size_t bytesTransferred);

    void
    onReadReady(sys::error_code ec, std::size_t bytesTransferred);

    void
    onReadDone(sys::error_code ec,  std::size_t bytesTransferred);

    void
    onShutdownDone(sys::error_code ec);

private:
    tcp::resolver _resolver;
    beast::ssl_stream<beast::tcp_stream> _stream;
    http::request<http::empty_body> _request;
    http::response<http::string_body> _response;
    beast::flat_buffer _buffer;

    long _fileSize{0};
    long _fileOffset{0};
    std::unique_ptr<int8_t[]> _fileData;

    RecognitionCalback _callback;
};

} // namespace jar
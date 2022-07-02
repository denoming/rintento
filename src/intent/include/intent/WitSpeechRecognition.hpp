#pragma once

#include "intent/WitCommon.hpp"
#include "intent/WitRecognition.hpp"

#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

namespace jar {

class WitSpeechRecognition : public WitRecognition,
                             public std::enable_shared_from_this<WitSpeechRecognition> {
public:
    using Ptr = std::shared_ptr<WitSpeechRecognition>;

    static Ptr
    create(ssl::context& context, net::any_io_executor& executor);

    void
    run(std::string_view host, std::string_view port, std::string_view auth, fs::path data);

private:
    explicit WitSpeechRecognition(ssl::context& context, net::any_io_executor& executor);

    void
    onResolveDone(sys::error_code error, const tcp::resolver::results_type& result);

    void
    onConnectDone(sys::error_code error,
                  const tcp::resolver::results_type::endpoint_type& endpoint);

    void
    onHandshakeDone(sys::error_code error);

    void
    onReadContinueDone(sys::error_code error, std::size_t bytesTransferred);

    void
    onWriteDone(sys::error_code error, std::size_t bytesTransferred);

    void
    onReadReady(sys::error_code error, std::size_t bytesTransferred);

    void
    onReadDone(sys::error_code error, std::size_t bytesTransferred);

    void
    onShutdownDone(sys::error_code error);

private:
    tcp::resolver _resolver;
    beast::ssl_stream<beast::tcp_stream> _stream;
    http::request<http::empty_body> _request;
    http::response<http::string_body> _response;
    beast::flat_buffer _buffer;
    long _fileSize{0};
    long _fileOffset{0};
    std::unique_ptr<int8_t[]> _fileData;
};

} // namespace jar
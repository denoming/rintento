#pragma once

#include "intent/Http.hpp"
#include "intent/WitRecognition.hpp"

#include <memory>
#include <string_view>
#include <filesystem>

namespace fs = std::filesystem;

namespace jar {

class WitSpeechRecognition : public WitRecognition,
                             public std::enable_shared_from_this<WitSpeechRecognition> {
public:
    using Ptr = std::shared_ptr<WitSpeechRecognition>;

    static Ptr
    create(ssl::context& context, net::any_io_executor executor);

    void
    run();

    void
    run(std::string_view host, std::string_view port, std::string_view auth);

    void
    feed(net::const_buffer buffer);

    void
    finalize();

private:
    explicit WitSpeechRecognition(ssl::context& context, net::any_io_executor executor);

    void
    resolve(std::string_view host, std::string_view port);

    void
    onResolveDone(sys::error_code error, const tcp::resolver::results_type& result);

    void
    connect(const tcp::resolver::results_type& addresses);

    void
    onConnectDone(sys::error_code error,
                  const tcp::resolver::results_type::endpoint_type& endpoint);

    void
    handshake();

    void
    onHandshakeDone(sys::error_code error);

    void
    readContinue();

    void
    onReadContinueDone(sys::error_code error, std::size_t bytesTransferred);

    void
    writeNextChunk(net::const_buffer buffer);

    void
    onWriteNextChunkDone(sys::error_code error, std::size_t bytesTransferred);

    void
    writeLastChunk();

    void
    onWriteLastChunkDone(sys::error_code error, std::size_t bytesTransferred);

    void
    read();

    void
    onReadDone(sys::error_code error, std::size_t bytesTransferred);

    void
    shutdown();

    void
    onShutdownDone(sys::error_code error);

private:
    net::any_io_executor _executor;
    tcp::resolver _resolver;
    beast::ssl_stream<beast::tcp_stream> _stream;
    http::request<http::empty_body> _request;
    http::response<http::string_body> _response;
    beast::flat_buffer _buffer;
};

} // namespace jar
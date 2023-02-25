#pragma once

#include "intent/WitRecognition.hpp"
#include "jarvis/Network.hpp"

#include <memory>
#include <string_view>

namespace jar {

class Config;

class WitSpeechRecognition : public WitRecognition,
                             public std::enable_shared_from_this<WitSpeechRecognition> {
public:
    static std::shared_ptr<WitSpeechRecognition>
    create(std::shared_ptr<Config> config, ssl::context& context, io::any_io_executor executor);

    void
    run();

    void
    run(std::string_view host, std::uint16_t port, std::string_view auth);

    void
    feed(io::const_buffer buffer);

    void
    finalize();

private:
    explicit WitSpeechRecognition(std::shared_ptr<Config> config,
                                  ssl::context& context,
                                  io::any_io_executor executor);

    void
    resolve(std::string_view host, std::uint16_t);

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
    writeNextChunk(io::const_buffer buffer);

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
    std::shared_ptr<Config> _config;
    io::any_io_executor _executor;
    tcp::resolver _resolver;
    beast::ssl_stream<beast::tcp_stream> _stream;
    http::request<http::empty_body> _req;
    http::response<http::string_body> _res;
    beast::flat_buffer _buf;
};

} // namespace jar
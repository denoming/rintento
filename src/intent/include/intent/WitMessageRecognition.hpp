#pragma once

#include "intent/Http.hpp"
#include "intent/WitRecognition.hpp"

#include <string>
#include <memory>

namespace jar {

class WitMessageRecognition : public WitRecognition,
                              public std::enable_shared_from_this<WitMessageRecognition> {
public:
    using Ptr = std::shared_ptr<WitMessageRecognition>;

    static Ptr
    create(ssl::context& context, net::any_io_executor& executor);

    void
    run();

    void
    run(std::string_view host, std::string_view port, std::string_view auth);

    void
    feed(std::string_view message);

private:
    explicit WitMessageRecognition(ssl::context& context, net::any_io_executor& executor);

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
    write();

    void
    onWriteDone(sys::error_code error, std::size_t bytesTransferred);

    void
    read();

    void
    onReadDone(sys::error_code error, std::size_t bytesTransferred);

    void
    shutdown();

    void
    onShutdownDone(sys::error_code error);

private:
    tcp::resolver _resolver;
    beast::ssl_stream<beast::tcp_stream> _stream;
    beast::flat_buffer _buffer;
    http::request<http::empty_body> _request;
    http::response<http::string_body> _response;
};

} // namespace jar
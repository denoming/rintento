#pragma once

#include "intent/WitCommon.hpp"
#include "intent/WitIntentSession.hpp"

#include <string>
#include <memory>

namespace jar {

class WitIntentMessageSession : public WitIntentSession,
                                public std::enable_shared_from_this<WitIntentMessageSession> {
public:
    using Ptr = std::shared_ptr<WitIntentMessageSession>;

    void
    run(std::string_view host,
        std::string_view port,
        std::string_view auth,
        std::string_view message);

    void
    cancel() override;

private:
    friend class WitIntentRecognizer;
    explicit WitIntentMessageSession(ssl::context& context, net::any_io_executor& executor);

    static Ptr
    create(ssl::context& context, net::any_io_executor& executor);

    void
    onResolveDone(sys::error_code ec, const tcp::resolver::results_type& result);

    void
    onConnectDone(sys::error_code ec, const tcp::resolver::results_type::endpoint_type& endpoint);

    void
    onHandshakeDone(sys::error_code ec);

    void
    onWriteDone(sys::error_code ec, std::size_t bytesTransferred);

    void
    onReadDone(sys::error_code ec, std::size_t bytesTransferred);

    void
    onShutdownDone(sys::error_code ec);

private:
    tcp::resolver _resolver;
    beast::ssl_stream<beast::tcp_stream> _stream;
    beast::flat_buffer _buffer;
    http::request<http::empty_body> _request;
    http::response<http::string_body> _response;
};

} // namespace jar
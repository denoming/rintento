#pragma once

#include "common/Types.hpp"

#include <jarvisto/Asio.hpp>
#include <jarvisto/Http.hpp>

#include <functional>
#include <memory>

namespace jar {

class RecognitionHandler {
public:
    using Stream = beast::tcp_stream;
    using Buffer = beast::flat_buffer;
    using Parser = http::request_parser<http::empty_body>;

    explicit RecognitionHandler(Stream& stream);

    virtual ~RecognitionHandler() = default;

    void
    setNext(std::shared_ptr<RecognitionHandler> handler);

    virtual io::awaitable<RecognitionResult>
    handle();

protected:
    io::awaitable<void>
    sendResponse(const RecognitionResult& result);

    io::awaitable<void>
    sendResponse(std::error_code ec);

    [[nodiscard]] Stream&
    stream();

    io::any_io_executor
    executor();

private:
    Stream& _stream;
    std::shared_ptr<RecognitionHandler> _next;
};

} // namespace jar
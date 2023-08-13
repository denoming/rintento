#pragma once

#include "intent/WitTypes.hpp"

#include <jarvisto/Network.hpp>

#include <functional>
#include <memory>

namespace jar {

class RecognitionHandler {
public:
    using OnDone = void(wit::Utterances, std::error_code);

    using Stream = beast::tcp_stream;
    using Buffer = beast::flat_buffer;
    using Parser = http::request_parser<http::empty_body>;

    explicit RecognitionHandler(Stream& stream);

    virtual ~RecognitionHandler() = default;

    void
    onDone(std::move_only_function<OnDone> callback);

    void
    setNext(std::shared_ptr<RecognitionHandler> handler);

    virtual void
    handle();

protected:
    void
    submit(wit::Utterances result);

    void
    submit(std::error_code error);

    void
    sendResponse(const wit::Utterances& result);

    void
    sendResponse(std::error_code ec);

    [[nodiscard]] Stream&
    stream();

    io::any_io_executor
    executor();

private:
    Stream& _stream;
    std::shared_ptr<RecognitionHandler> _next;
    std::move_only_function<OnDone> _onDone;
};

} // namespace jar
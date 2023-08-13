#pragma once

#include "intent/WitTypes.hpp"

#include <jarvisto/Network.hpp>

#include <functional>
#include <memory>

namespace jar {

class RecognitionHandler;
class WitRecognitionFactory;

class RecognitionSession : public std::enable_shared_from_this<RecognitionSession> {
public:
    using OnComplete = void(std::size_t id);

    [[nodiscard]] static std::shared_ptr<RecognitionSession>
    create(std::size_t id, tcp::socket&& socket, std::shared_ptr<WitRecognitionFactory> factory);

    void
    onComplete(std::move_only_function<OnComplete> callback);

    [[nodiscard]] std::size_t
    id() const;

    void
    run();

private:
    RecognitionSession(std::size_t id,
                       tcp::socket&& socket,
                       std::shared_ptr<WitRecognitionFactory> factory);

    void
    doReadHeader();

    void
    onReadHeaderDone(sys::error_code ec, std::size_t bytes);

    void
    onComplete(wit::Utterances utterances, std::error_code ec);

    std::shared_ptr<RecognitionHandler>
    getHandler();

    void
    finalize();

private:
    std::size_t _id;
    beast::tcp_stream _stream;
    beast::flat_buffer _buffer;
    http::request_parser<http::empty_body> _parser;
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<RecognitionHandler> _handler;
    std::move_only_function<OnComplete> _onComplete;
};

} // namespace jar
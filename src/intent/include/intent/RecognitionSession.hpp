#pragma once

#include <jarvisto/network/Asio.hpp>
#include <jarvisto/network/Http.hpp>

#include <memory>

namespace jar {

class IRecognitionFactory;
class RecognitionHandler;
class AutomationPerformer;

class RecognitionSession : public std::enable_shared_from_this<RecognitionSession> {
public:
    using Ptr = std::shared_ptr<RecognitionSession>;

    [[nodiscard]] static Ptr
    create(std::size_t id,
           tcp::socket&& socket,
           std::shared_ptr<IRecognitionFactory> factory,
           std::shared_ptr<AutomationPerformer> performer);

    [[nodiscard]] std::size_t
    id() const;

    void
    run();

private:
    RecognitionSession(std::size_t id,
                       tcp::socket&& socket,
                       std::shared_ptr<IRecognitionFactory> factory,
                       std::shared_ptr<AutomationPerformer> performer);

    io::awaitable<void>
    doRun();

    std::shared_ptr<RecognitionHandler>
    getHandler();

private:
    std::size_t _id;
    beast::tcp_stream _stream;
    beast::flat_buffer _buffer;
    http::request_parser<http::empty_body> _parser;
    std::shared_ptr<IRecognitionFactory> _factory;
    std::shared_ptr<AutomationPerformer> _performer;
};

} // namespace jar
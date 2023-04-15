#pragma once

#include "intent/Types.hpp"
#include "jarvis/Network.hpp"

#include <functional>
#include <memory>

namespace jar {

class IntentPerformer;
class RecognitionHandler;
class RecognitionConnection;
class WitRecognitionFactory;

class RecognitionDispatcher : public std::enable_shared_from_this<RecognitionDispatcher> {
public:
    using OnDone = void(std::uint16_t identity);

    [[nodiscard]] static std::shared_ptr<RecognitionDispatcher>
    create(uint16_t id,
           std::shared_ptr<RecognitionConnection> connection,
           std::shared_ptr<IntentPerformer> executor,
           std::shared_ptr<WitRecognitionFactory> factory);

    void
    onDone(std::move_only_function<OnDone> callback);

    [[nodiscard]] uint16_t
    id() const;

    void
    dispatch();

private:
    RecognitionDispatcher(uint16_t id,
                          std::shared_ptr<RecognitionConnection> connection,
                          std::shared_ptr<IntentPerformer> performer,
                          std::shared_ptr<WitRecognitionFactory> factory);

    void
    readHeader();

    void
    onReadHeaderDone(beast::flat_buffer& buffer,
                     http::request_parser<http::empty_body>& parser,
                     std::error_code error);

    void
    onDone(UtteranceSpecs utterances, std::error_code error);

    std::shared_ptr<RecognitionHandler>
    getHandler();

    void
    finalize();

private:
    uint16_t _id;
    std::shared_ptr<RecognitionConnection> _connection;
    std::shared_ptr<IntentPerformer> _performer;
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<RecognitionHandler> _handler;
    std::move_only_function<OnDone> _onDone;
};

} // namespace jar
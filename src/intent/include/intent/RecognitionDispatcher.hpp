#pragma once

#include "intent/IntentPerformer.hpp"
#include "intent/RecognitionConnection.hpp"
#include "intent/RecognitionHandler.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <memory>
#include <functional>

namespace jar {

class RecognitionDispatcher : public std::enable_shared_from_this<RecognitionDispatcher> {
public:
    using Ptr = std::shared_ptr<RecognitionDispatcher>;

    using DoneSignature = void(std::uint16_t identity);

    static Ptr
    create(uint16_t identity,
           RecognitionConnection::Ptr connection,
           IntentPerformer::Ptr executor,
           WitRecognitionFactory::Ptr factory);

    uint16_t
    identity() const;

    void
    whenDone(std::function<DoneSignature> callback);

    void
    dispatch();

private:
    RecognitionDispatcher(uint16_t identity,
                          RecognitionConnection::Ptr connection,
                              IntentPerformer::Ptr performer,
                              WitRecognitionFactory::Ptr factory);

    void
    readHeader();

    void
    onReadHeaderDone(beast::flat_buffer& buffer,
                     http::request_parser<http::empty_body>& parser,
                     sys::error_code error);

    void
    onComplete(Utterances utterances, sys::error_code error);

    RecognitionHandler::Ptr
    getHandler();

    void
    finalize();

private:
    uint16_t _identity;
    RecognitionConnection::Ptr _connection;
    IntentPerformer::Ptr _performer;
    WitRecognitionFactory::Ptr _factory;
    RecognitionHandler::Ptr _handler;
    std::function<DoneSignature> _doneCallback;
};

} // namespace jar
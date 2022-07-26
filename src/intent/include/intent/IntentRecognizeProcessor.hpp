#pragma once

#include "intent/IntentPerformer.hpp"
#include "intent/IntentRecognizeConnection.hpp"
#include "intent/IntentRecognizeHandler.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <memory>

namespace jar {

class IntentRecognizeProcessor : public std::enable_shared_from_this<IntentRecognizeProcessor> {
public:
    using Ptr = std::shared_ptr<IntentRecognizeProcessor>;

    static Ptr
    create(IntentRecognizeConnection::Ptr connection,
           IntentPerformer::Ptr executor,
           WitRecognitionFactory::Ptr factory);

    void
    process();

private:
    IntentRecognizeProcessor(IntentRecognizeConnection::Ptr connection,
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

    IntentRecognizeHandler::Ptr
    getHandler();

private:
    IntentRecognizeConnection::Ptr _connection;
    IntentPerformer::Ptr _performer;
    WitRecognitionFactory::Ptr _factory;
    IntentRecognizeHandler::Ptr _handler;
};

} // namespace jar
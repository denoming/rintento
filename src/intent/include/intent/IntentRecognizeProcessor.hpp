#pragma once

#include "intent/IntentRecognizeStrategy.hpp"
#include "intent/IntentPerformer.hpp"
#include "intent/IntentRecognizeConnection.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <memory>

namespace jar {

class IntentRecognizeProcessor : public std::enable_shared_from_this<IntentRecognizeProcessor> {
public:
    using Ptr = std::shared_ptr<IntentRecognizeProcessor>;

    static Ptr
    create(IntentRecognizeConnection::Ptr connection,
           IntentPerformer::Ptr executor,
           WitRecognitionFactory& factory);

    void
    setStrategy(IntentRecognizeStrategy::Ptr strategy);

    void
    process();

private:
    friend class Dispatcher;

    IntentRecognizeProcessor(IntentRecognizeConnection::Ptr connection,
                             IntentPerformer::Ptr performer,
                             WitRecognitionFactory& factory);

    IntentRecognizeConnection::Ptr
    connection();

    void
    read();

    void
    onReadDone(sys::error_code error, http::request<http::empty_body> request);

    void
    onComplete(Utterances utterances, sys::error_code error);

    void
    onComplete();

private:
    IntentRecognizeConnection::Ptr _connection;
    IntentPerformer::Ptr _performer;
    WitRecognitionFactory& _factory;
    IntentRecognizeStrategy::Ptr _strategy;
};

} // namespace jar
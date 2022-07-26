#pragma once

#include "intent/IntentRecognizeConnection.hpp"
#include "intent/IntentRecognizeHandler.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitRecognitionObserver.hpp"

namespace jar {

class IntentRecognizeMessageHandler final : public IntentRecognizeHandler {
public:
    IntentRecognizeMessageHandler(IntentRecognizeConnection::Ptr connection,
                                  WitRecognitionFactory::Ptr factory,
                                  Callback callback);

    ~IntentRecognizeMessageHandler() override;

    void
    handle(Buffer& buffer, Parser& parser) override;

private:
    bool
    canHandle(const Parser::value_type& request) const;

    void
    onRecognitionData(std::string message);

    void
    onRecognitionComplete(Utterances result, sys::error_code error);

private:
    WitRecognitionFactory::Ptr _factory;
    WitMessageRecognition::Ptr _recognition;
    WitRecognitionObserver::Ptr _observer;
    signals::connection _onDataCon;
};

} // namespace jar
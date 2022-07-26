#pragma once

#include "intent/IntentRecognizeConnection.hpp"
#include "intent/IntentRecognizeHandler.hpp"
#include "intent/IntentSpeechBuffer.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "intent/WitSpeechRecognition.hpp"
#include "intent/WitRecognitionObserver.hpp"

#include <boost/circular_buffer.hpp>

#include <mutex>

namespace jar {

class IntentRecognizeSpeechHandler final : public IntentRecognizeHandler {
public:
    IntentRecognizeSpeechHandler(IntentRecognizeConnection::Ptr connection,
                                 WitRecognitionFactory::Ptr factory,
                                 Callback callback);

    ~IntentRecognizeSpeechHandler() override;

    void
    handle(Buffer& buffer, Parser& parser) override;

private:
    bool
    canHandle(const Parser::value_type& request) const;

    void
    onRecognitionData();

    void
    onRecognitionComplete(Utterances result, sys::error_code error);

private:
    WitRecognitionFactory::Ptr _factory;
    WitSpeechRecognition::Ptr _recognition;
    WitRecognitionObserver::Ptr _observer;
    signals::connection _onDataCon;
    IntentSpeechBuffer _speechData;
};

} // namespace jar
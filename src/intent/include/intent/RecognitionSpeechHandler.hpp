#pragma once

#include "intent/RecognitionConnection.hpp"
#include "intent/RecognitionHandler.hpp"
#include "intent/SpeechDataBuffer.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "intent/WitSpeechRecognition.hpp"
#include "intent/WitRecognitionObserver.hpp"

#include <boost/circular_buffer.hpp>

#include <mutex>

namespace jar {

class RecognitionSpeechHandler final : public RecognitionHandler {
public:
    RecognitionSpeechHandler(RecognitionConnection::Ptr connection,
                             WitRecognitionFactory::Ptr factory,
                             Callback callback);

    ~RecognitionSpeechHandler() override;

    void
    handle(Buffer& buffer, Parser& parser) override;

private:
    bool
    canHandle(const Parser::value_type& request) const;

    void
    onRecognitionData();

    void
    onRecognitionError(sys::error_code error);

    void
    onRecognitionSuccess(Utterances result);

private:
    WitRecognitionFactory::Ptr _factory;
    WitSpeechRecognition::Ptr _recognition;
    WitRecognitionObserver::Ptr _observer;
    signals::connection _onDataCon;
    SpeechDataBuffer _speechData;
};

} // namespace jar
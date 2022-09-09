#pragma once

#include "intent/RecognitionConnection.hpp"
#include "intent/RecognitionHandler.hpp"
#include "intent/SpeechDataBuffer.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "intent/WitSpeechRecognition.hpp"

#include <boost/circular_buffer.hpp>

#include <memory>

namespace jar {

class RecognitionSpeechHandler final
    : public RecognitionHandler,
      public std::enable_shared_from_this<RecognitionSpeechHandler> {
public:
    [[nodiscard]] static Ptr
    create(RecognitionConnection::Ptr connection,
           WitRecognitionFactory::Ptr factory,
           Callback callback);

    void
    handle(Buffer& buffer, Parser& parser) final;

private:
    RecognitionSpeechHandler(RecognitionConnection::Ptr connection,
                             WitRecognitionFactory::Ptr factory,
                             Callback callback);

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
    SpeechDataBuffer _speechData;
};

} // namespace jar
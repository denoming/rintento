#pragma once

#include "intent/RecognitionHandler.hpp"
#include "intent/SpeechDataBuffer.hpp"

#include <memory>

namespace jar {

class WitRecognitionFactory;
class WitSpeechRecognition;

class RecognitionSpeechHandler final
    : public RecognitionHandler,
      public std::enable_shared_from_this<RecognitionSpeechHandler> {
public:
    [[nodiscard]] static std::shared_ptr<RecognitionHandler>
    create(Stream& stream,
           Buffer& buffer,
           Parser& parser,
           std::shared_ptr<WitRecognitionFactory> factory);

    void
    handle() final;

private:
    RecognitionSpeechHandler(Stream& stream,
                             Buffer& buffer,
                             Parser& parser,
                             std::shared_ptr<WitRecognitionFactory> factory);

    [[nodiscard]] bool
    canHandle() const;

    [[nodiscard]] std::shared_ptr<WitSpeechRecognition>
    createRecognition();

    void
    handleSpeechData();

    void
    onRecognitionData();

    void
    onRecognitionError(std::error_code error);

    void
    onRecognitionSuccess(wit::Utterances result);

private:
    Buffer& _buffer;
    Parser& _parser;
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<WitSpeechRecognition> _recognition;
    SpeechDataBuffer _speechData;
};

} // namespace jar
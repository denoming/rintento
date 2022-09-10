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
    create(std::shared_ptr<RecognitionConnection> connection,
           std::shared_ptr<WitRecognitionFactory> factory);

    void
    handle(Buffer& buffer, Parser& parser) final;

private:
    RecognitionSpeechHandler(std::shared_ptr<RecognitionConnection> connection,
                             std::shared_ptr<WitRecognitionFactory> factory);

    [[nodiscard]] bool
    canHandle(const Parser::value_type& request) const;

    [[nodiscard]] std::shared_ptr<WitSpeechRecognition>
    createRecognition();

    void
    handleSpeechData(Buffer& buffer, Parser& parser);

    void
    onRecognitionData();

    void
    onRecognitionError(sys::error_code error);

    void
    onRecognitionSuccess(Utterances result);

private:
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<WitSpeechRecognition> _recognition;
    SpeechDataBuffer _speechData;
};

} // namespace jar
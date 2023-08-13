#pragma once

#include "intent/RecognitionHandler.hpp"

#include <memory>
#include <string>

namespace jar {

class RecognitionConnection;
class WitRecognitionFactory;
class WitMessageRecognition;

class RecognitionMessageHandler final
    : public RecognitionHandler,
      public std::enable_shared_from_this<RecognitionMessageHandler> {
public:
    [[nodiscard]] static std::shared_ptr<RecognitionMessageHandler>
    create(Stream& stream,
           Buffer& buffer,
           Parser& parser,
           std::shared_ptr<WitRecognitionFactory> factory);

    void
    handle() final;

private:
    RecognitionMessageHandler(Stream& stream,
                              Buffer& buffer,
                              Parser& parser,
                              std::shared_ptr<WitRecognitionFactory> factory);

    [[nodiscard]] bool
    canHandle() const;

    [[nodiscard]] std::shared_ptr<WitMessageRecognition>
    createRecognition();

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
    std::shared_ptr<WitMessageRecognition> _recognition;
    std::string _message;
};

} // namespace jar
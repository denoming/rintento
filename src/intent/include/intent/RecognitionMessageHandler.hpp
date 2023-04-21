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
    create(std::shared_ptr<RecognitionConnection> connection,
           std::shared_ptr<WitRecognitionFactory> factory);

    void
    handle(Buffer& buffer, Parser& parser) final;

private:
    RecognitionMessageHandler(std::shared_ptr<RecognitionConnection> connection,
                              std::shared_ptr<WitRecognitionFactory> factory);

    [[nodiscard]] bool
    canHandle(const Parser::value_type& request) const;

    [[nodiscard]] std::shared_ptr<WitMessageRecognition>
    createRecognition();

    void
    onRecognitionData();

    void
    onRecognitionError(std::error_code error);

    void
    onRecognitionSuccess(Utterances result);

private:
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<WitMessageRecognition> _recognition;
    std::string _message;
};

} // namespace jar
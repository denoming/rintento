#pragma once

#include "intent/RecognitionConnection.hpp"
#include "intent/RecognitionHandler.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <memory>

namespace jar {

class RecognitionMessageHandler final
    : public RecognitionHandler,
      public std::enable_shared_from_this<RecognitionMessageHandler> {
public:
    [[nodiscard]] static Ptr
    create(RecognitionConnection::Ptr connection,
           WitRecognitionFactory::Ptr factory,
           Callback callback);

    void
    handle(Buffer& buffer, Parser& parser) final;

private:
    RecognitionMessageHandler(RecognitionConnection::Ptr connection,
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
    WitMessageRecognition::Ptr _recognition;
    std::string _message;
};

} // namespace jar
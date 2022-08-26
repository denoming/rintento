#pragma once

#include "intent/RecognitionConnection.hpp"
#include "intent/RecognitionHandler.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitRecognitionObserver.hpp"

namespace jar {

class RecognitionMessageHandler final : public RecognitionHandler {
public:
    RecognitionMessageHandler(RecognitionConnection::Ptr connection,
                                  WitRecognitionFactory::Ptr factory,
                                  Callback callback);

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
    WitMessageRecognition::Ptr _recognition;
    WitRecognitionObserver::Ptr _observer;
    std::string _message;
};

} // namespace jar
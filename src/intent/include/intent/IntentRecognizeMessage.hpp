#pragma once

#include "intent/IntentRecognizeConnection.hpp"
#include "intent/IntentRecognizeStrategy.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/RecognitionObserver.hpp"

namespace jar {

class IntentRecognizeMessage : public IntentRecognizeStrategy {
public:
    IntentRecognizeMessage(WitMessageRecognition::Ptr recognition,
                           IntentRecognizeConnection::Ptr connection,
                           std::string_view message);

    void
    execute(Callback callback) override;

private:
    void onComplete(Utterances result, sys::error_code error);

private:
    WitMessageRecognition::Ptr _recognition;
    IntentRecognizeConnection::Ptr _connection;
    std::string _message;
    Callback _callback;
    RecognitionObserver::Ptr _observer;
};

} // namespace jar
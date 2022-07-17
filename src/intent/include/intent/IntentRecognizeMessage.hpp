#pragma once

#include "intent/IntentRecognizeConnection.hpp"
#include "intent/IntentRecognizeStrategy.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/RecognitionObserver.hpp"

#include <boost/signals2/connection.hpp>

namespace signals = boost::signals2;

namespace jar {

class IntentRecognizeMessage : public IntentRecognizeStrategy {
public:
    IntentRecognizeMessage(WitMessageRecognition::Ptr recognition,
                           IntentRecognizeConnection::Ptr connection,
                           std::string message);

    ~IntentRecognizeMessage();

    void
    execute(Callback callback) override;

private:
    void
    onComplete(Utterances result, sys::error_code error);

    void
    onData();

private:
    WitMessageRecognition::Ptr _recognition;
    IntentRecognizeConnection::Ptr _connection;
    std::string _message;
    Callback _callback;
    RecognitionObserver::Ptr _observer;
    signals::connection _onDataCon;
};

} // namespace jar
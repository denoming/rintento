#pragma once

#include "intent/IntentRecognizeConnection.hpp"
#include "intent/IntentRecognizeStrategy.hpp"
#include "intent/WitSpeechRecognition.hpp"

namespace jar {

class IntentRecognizeSpeech : public IntentRecognizeStrategy {
public:
    IntentRecognizeSpeech(WitSpeechRecognition::Ptr recognition,
                          IntentRecognizeConnection::Ptr connection);

    void
    execute(Callback callback) override;

private:
    WitSpeechRecognition::Ptr _recognition;
    IntentRecognizeConnection::Ptr _connection;
};

} // namespace jar
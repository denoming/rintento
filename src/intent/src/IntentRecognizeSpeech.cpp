#include "intent/IntentRecognizeSpeech.hpp"

namespace jar {

IntentRecognizeSpeech::IntentRecognizeSpeech(WitSpeechRecognition::Ptr recognition,
                                             IntentRecognizeConnection::Ptr connection)
    : _recognition{std::move(recognition)}
    , _connection{std::move(connection)}
{
}

void
IntentRecognizeSpeech::execute(Callback callback)
{
}

} // namespace jar
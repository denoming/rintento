#pragma once

#include "intent/IntentRecognizeConnection.hpp"
#include "intent/IntentRecognizeHandler.hpp"

namespace jar {

class IntentRecognizeTerminalHandler : public IntentRecognizeHandler {
public:
    IntentRecognizeTerminalHandler(IntentRecognizeConnection::Ptr connection, Callback callback);

    void
    handle(Buffer& buffer, Parser& parser) override;
};

} // namespace jar
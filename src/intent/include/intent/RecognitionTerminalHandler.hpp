#pragma once

#include "intent/RecognitionConnection.hpp"
#include "intent/RecognitionHandler.hpp"

namespace jar {

class RecognitionTerminalHandler : public RecognitionHandler {
public:
    RecognitionTerminalHandler(RecognitionConnection::Ptr connection, Callback callback);

    void
    handle(Buffer& buffer, Parser& parser) override;
};

} // namespace jar
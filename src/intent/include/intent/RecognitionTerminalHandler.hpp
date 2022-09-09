#pragma once

#include "intent/RecognitionConnection.hpp"
#include "intent/RecognitionHandler.hpp"

#include <memory>

namespace jar {

class RecognitionTerminalHandler : public RecognitionHandler,
                                   public std::enable_shared_from_this<RecognitionTerminalHandler> {
public:
    [[nodiscard]] static Ptr
    create(RecognitionConnection::Ptr connection, Callback callback);

    void
    handle(Buffer& buffer, Parser& parser) override;

private:
    RecognitionTerminalHandler(RecognitionConnection::Ptr connection, Callback callback);
};

} // namespace jar
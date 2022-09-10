#pragma once

#include "intent/RecognitionHandler.hpp"

#include <memory>

namespace jar {

class RecognitionConnection;

class RecognitionTerminalHandler final
    : public RecognitionHandler,
      public std::enable_shared_from_this<RecognitionTerminalHandler> {
public:
    [[nodiscard]] static std::shared_ptr<RecognitionHandler>
    create(std::shared_ptr<RecognitionConnection> connection);

    void
    handle(Buffer& buffer, Parser& parser) final;

private:
    RecognitionTerminalHandler(std::shared_ptr<RecognitionConnection> connection);
};

} // namespace jar
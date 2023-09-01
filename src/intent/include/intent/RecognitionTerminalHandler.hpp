#pragma once

#include "intent/RecognitionHandler.hpp"

#include <memory>

namespace jar {

class RecognitionTerminalHandler final
    : public RecognitionHandler,
      public std::enable_shared_from_this<RecognitionTerminalHandler> {
public:
    [[nodiscard]] static std::shared_ptr<RecognitionHandler>
    create(Stream& stream);

    io::awaitable<wit::Utterances>
    handle() final;

private:
    explicit RecognitionTerminalHandler(Stream& stream);
};

} // namespace jar
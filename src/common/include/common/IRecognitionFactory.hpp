#pragma once

#include "common/Recognition.hpp"
#include "coro/BoundedChannel.hpp"

#include <memory>

namespace jar {

class IRecognitionFactory {
public:
    using DataChannel = coro::BoundedChannel<char>;

    virtual ~IRecognitionFactory() = default;

    [[nodiscard]] virtual bool
    canRecognizeMessage() const
        = 0;

    [[nodiscard]] virtual std::shared_ptr<Recognition>
    message(io::any_io_executor executor, std::shared_ptr<DataChannel> channel) = 0;

    [[nodiscard]] virtual bool
    canRecognizeSpeech() const
        = 0;

    [[nodiscard]] virtual std::shared_ptr<Recognition>
    speech(io::any_io_executor executor, std::shared_ptr<DataChannel> channel) = 0;
};

} // namespace jar
#pragma once

#include "coro/BoundedChannel.hpp"
#include "intent/RecognitionHandler.hpp"

#include <memory>

namespace jar {

class IRecognitionFactory;

class RecognitionSpeechHandler final
    : public RecognitionHandler,
      public std::enable_shared_from_this<RecognitionSpeechHandler> {
public:
    using Ptr = std::shared_ptr<RecognitionSpeechHandler>;
    using Channel = coro::BoundedChannel<char>;

    [[nodiscard]] static Ptr
    create(Stream& stream,
           Buffer& buffer,
           Parser& parser,
           std::shared_ptr<IRecognitionFactory> factory);

    io::awaitable<RecognitionResult>
    handle() final;

private:
    RecognitionSpeechHandler(Stream& stream,
                             Buffer& buffer,
                             Parser& parser,
                             std::shared_ptr<IRecognitionFactory> factory);

    [[nodiscard]] bool
    canHandle() const;

    io::awaitable<void>
    sendSpeechData(std::shared_ptr<Channel> channel);

private:
    Buffer& _buffer;
    Parser& _parser;
    std::shared_ptr<IRecognitionFactory> _factory;
};

} // namespace jar
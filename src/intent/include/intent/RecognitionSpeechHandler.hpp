#pragma once

#include "coro/BoundedDataChannel.hpp"
#include "intent/RecognitionHandler.hpp"

#include <memory>

namespace jar {

class WitRecognitionFactory;
class WitSpeechRecognition;

class RecognitionSpeechHandler final
    : public RecognitionHandler,
      public std::enable_shared_from_this<RecognitionSpeechHandler> {
public:
    using Ptr = std::shared_ptr<RecognitionSpeechHandler>;
    using Channel = coro::BoundedDataChannel<char>;

    [[nodiscard]] static Ptr
    create(Stream& stream,
           Buffer& buffer,
           Parser& parser,
           std::shared_ptr<WitRecognitionFactory> factory);

    io::awaitable<wit::Utterances>
    handle() final;

private:
    RecognitionSpeechHandler(Stream& stream,
                             Buffer& buffer,
                             Parser& parser,
                             std::shared_ptr<WitRecognitionFactory> factory);

    [[nodiscard]] bool
    canHandle() const;

    io::awaitable<void>
    sendSpeechData(std::shared_ptr<Channel> channel);

private:
    Buffer& _buffer;
    Parser& _parser;
    std::shared_ptr<WitRecognitionFactory> _factory;
};

} // namespace jar
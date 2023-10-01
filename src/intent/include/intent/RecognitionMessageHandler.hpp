#pragma once

#include "coro/BoundedDataChannel.hpp"
#include "intent/RecognitionHandler.hpp"

#include <memory>
#include <string>

namespace jar {

class IRecognitionFactory;

class RecognitionMessageHandler final
    : public RecognitionHandler,
      public std::enable_shared_from_this<RecognitionMessageHandler> {
public:
    using Ptr = std::shared_ptr<RecognitionMessageHandler>;
    using Channel = coro::BoundedDataChannel<char>;

    [[nodiscard]] static Ptr
    create(Stream& stream,
           Buffer& buffer,
           Parser& parser,
           std::shared_ptr<IRecognitionFactory> factory);

    io::awaitable<RecognitionResult>
    handle() final;

private:
    RecognitionMessageHandler(Stream& stream,
                              Buffer& buffer,
                              Parser& parser,
                              std::shared_ptr<IRecognitionFactory> factory);

    [[nodiscard]] bool
    canHandle() const;

    io::awaitable<void>
    sendMessageData(std::shared_ptr<Channel> channel);

private:
    Buffer& _buffer;
    Parser& _parser;
    std::shared_ptr<IRecognitionFactory> _factory;
};

} // namespace jar
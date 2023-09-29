#pragma once

#include "intent/RecognitionHandler.hpp"

#include <boost/asio/experimental/channel.hpp>

#include <memory>
#include <string>

namespace jar {

namespace wit {
class RecognitionFactory;
class MessageRecognition;
} // namespace wit

class RecognitionMessageHandler final
    : public RecognitionHandler,
      public std::enable_shared_from_this<RecognitionMessageHandler> {
public:
    using Ptr = std::shared_ptr<RecognitionMessageHandler>;
    using Channel = boost::asio::experimental::channel<void(sys::error_code, std::string)>;

    [[nodiscard]] static Ptr
    create(Stream& stream,
           Buffer& buffer,
           Parser& parser,
           std::shared_ptr<wit::RecognitionFactory> factory);

    io::awaitable<wit::Utterances>
    handle() final;

private:
    RecognitionMessageHandler(Stream& stream,
                              Buffer& buffer,
                              Parser& parser,
                              std::shared_ptr<wit::RecognitionFactory> factory);

    [[nodiscard]] bool
    canHandle() const;

    io::awaitable<void>
    sendMessageData(std::shared_ptr<Channel> channel);

private:
    Buffer& _buffer;
    Parser& _parser;
    std::shared_ptr<wit::RecognitionFactory> _factory;
    std::shared_ptr<wit::MessageRecognition> _recognition;
};

} // namespace jar
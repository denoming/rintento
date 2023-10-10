#pragma once

#include "coro/BoundedChannel.hpp"
#include "wit/RemoteRecognition.hpp"

#include <memory>

namespace jar::wit {

class MessageRecognition final : public RemoteRecognition,
                                 public std::enable_shared_from_this<MessageRecognition> {
public:
    using Ptr = std::shared_ptr<MessageRecognition>;
    using Channel = coro::BoundedChannel<char>;

    static Ptr
    create(io::any_io_executor executor,
           ssl::context& context,
           std::string host,
           std::string port,
           std::string auth,
           std::shared_ptr<Channel> channel);

private:
    explicit MessageRecognition(io::any_io_executor executor,
                                ssl::context& context,
                                std::string host,
                                std::string port,
                                std::string auth,
                                std::shared_ptr<Channel> channel);

    io::awaitable<Utterances>
    process() final;

private:
    std::shared_ptr<Channel> _channel;
};

} // namespace jar::wit
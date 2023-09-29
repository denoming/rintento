#pragma once

#include "wit/Recognition.hpp"

#include <boost/asio/experimental/channel.hpp>

#include <memory>

namespace jar::wit {

class MessageRecognition final : public Recognition,
                                 public std::enable_shared_from_this<MessageRecognition> {
public:
    using Ptr = std::shared_ptr<MessageRecognition>;
    using Channel = boost::asio::experimental::channel<void(sys::error_code, std::string)>;

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
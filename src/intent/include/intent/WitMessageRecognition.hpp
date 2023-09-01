#pragma once

#include "intent/WitRecognition.hpp"

#include <boost/asio/experimental/channel.hpp>

#include <memory>

namespace jar {

class WitMessageRecognition final : public WitRecognition,
                                    public std::enable_shared_from_this<WitMessageRecognition> {
public:
    using Ptr = std::shared_ptr<WitMessageRecognition>;
    using Channel = boost::asio::experimental::channel<void(sys::error_code, std::string)>;

    static Ptr
    create(io::any_io_executor executor,
           ssl::context& context,
           std::string host,
           std::string port,
           std::string auth,
           std::shared_ptr<Channel> channel);

private:
    explicit WitMessageRecognition(io::any_io_executor executor,
                                   ssl::context& context,
                                   std::string host,
                                   std::string port,
                                   std::string auth,
                                   std::shared_ptr<Channel> channel);

    io::awaitable<wit::Utterances>
    process() final;

private:
    std::shared_ptr<Channel> _channel;
};

} // namespace jar
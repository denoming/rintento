#pragma once

#include "wit/Recognition.hpp"

#include "coro/BoundedDataChannel.hpp"

#include <memory>
#include <string_view>

namespace jar::wit {

class SpeechRecognition final : public Recognition,
                                public std::enable_shared_from_this<SpeechRecognition> {
public:
    using Ptr = std::shared_ptr<SpeechRecognition>;
    using Channel = coro::BoundedDataChannel<char>;

    static Ptr
    create(io::any_io_executor executor,
           ssl::context& context,
           std::string host,
           std::string port,
           std::string auth,
           std::shared_ptr<Channel> channel);

private:
    explicit SpeechRecognition(io::any_io_executor executor,
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

} // namespace jar::wit
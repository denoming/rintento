#pragma once

#include "intent/WitRecognition.hpp"

#include "coro/BoundedDataChannel.hpp"

#include <memory>
#include <string_view>

namespace jar {

class WitSpeechRecognition final : public WitRecognition,
                                   public std::enable_shared_from_this<WitSpeechRecognition> {
public:
    using Ptr = std::shared_ptr<WitSpeechRecognition>;
    using Channel = coro::BoundedDataChannel<char>;

    static Ptr
    create(io::any_io_executor executor,
           ssl::context& context,
           std::string host,
           std::string port,
           std::string auth,
           std::shared_ptr<Channel> channel);

private:
    explicit WitSpeechRecognition(io::any_io_executor executor,
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
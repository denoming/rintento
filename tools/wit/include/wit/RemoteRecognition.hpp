#pragma once

#include "common/Recognition.hpp"
#include "wit/Types.hpp"

#include <jarvisto/Network.hpp>

namespace jar::wit {

class RemoteRecognition : public Recognition {
public:
    RemoteRecognition(io::any_io_executor executor,
                      ssl::context& context,
                      std::string remoteHost,
                      std::string remotePort,
                      std::string remoteAuth);

    io::awaitable<RecognitionResult>
    run() final;

protected:
    using Stream = beast::ssl_stream<beast::tcp_stream>;

    [[nodiscard]] Stream&
    stream();

    [[nodiscard]] const std::string&
    remoteHost() const;

    [[nodiscard]] const std::string&
    remotePort() const;

    [[nodiscard]] const std::string&
    remoteAuth() const;

    virtual io::awaitable<void>
    connect();

    virtual io::awaitable<wit::Utterances>
    process();

    virtual io::awaitable<void>
    shutdown();

private:
    Stream _stream;
    std::string _remoteHost;
    std::string _remotePort;
    std::string _remoteAuth;
};

} // namespace jar::wit
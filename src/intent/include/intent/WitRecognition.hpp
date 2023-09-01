#pragma once

#include "intent/WitTypes.hpp"

#include <jarvisto/Cancellable.hpp>
#include <jarvisto/Network.hpp>

namespace jar {

class WitRecognition : public Cancellable {
public:
    WitRecognition(io::any_io_executor executor,
                   ssl::context& context,
                   std::string host,
                   std::string port,
                   std::string auth);

    io::awaitable<wit::Utterances>
    run();

protected:
    using Stream = beast::ssl_stream<beast::tcp_stream>;

    [[nodiscard]] Stream&
    stream();

    [[nodiscard]] const std::string&
    host() const;

    [[nodiscard]] const std::string&
    port() const;

    [[nodiscard]] const std::string&
    auth() const;

    virtual io::awaitable<void>
    connect();

    virtual io::awaitable<wit::Utterances>
    process();

    virtual io::awaitable<void>
    shutdown();

private:
    Stream _stream;
    std::string _host;
    std::string _port;
    std::string _auth;
};

} // namespace jar
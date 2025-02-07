#include "intent/RecognitionServer.hpp"

#include "common/IRecognitionFactory.hpp"
#include "intent/AutomationPerformer.hpp"
#include "intent/RecognitionSession.hpp"

#include <jarvisto/core/Logger.hpp>

#include <functional>

namespace std {

template<>
struct hash<tcp::endpoint> {
    size_t
    operator()(const tcp::endpoint& e) const
    {
        size_t h1 = std::hash<std::string>{}(e.address().to_string());
        size_t h2 = std::hash<io::ip::port_type>{}(e.port());
        return h1 ^ (h2 << 1);
    }
};

} // namespace std

namespace {

std::optional<std::size_t>
getSessionId(const tcp::socket& socket)
{
    sys::error_code ec;
    const auto endpoint = socket.remote_endpoint(ec);
    if (ec) {
        return std::nullopt;
    }
    return std::hash<tcp::endpoint>{}(endpoint);
}

} // namespace

namespace jar {

std::shared_ptr<RecognitionServer>
RecognitionServer::create(io::any_io_executor executor,
                          std::shared_ptr<IRecognitionFactory> factory,
                          std::shared_ptr<AutomationPerformer> performer)
{
    return Ptr(
        new RecognitionServer{std::move(executor), std::move(factory), std::move(performer)});
}

RecognitionServer::RecognitionServer(io::any_io_executor executor,
                                     std::shared_ptr<IRecognitionFactory> factory,
                                     std::shared_ptr<AutomationPerformer> performer)
    : _executor{std::move(executor)}
    , _factory{std::move(factory)}
    , _performer{std::move(performer)}
{
}

void
RecognitionServer::listen(io::ip::port_type port)
{
    return listen(tcp::endpoint{tcp::v4(), port});
}

void
RecognitionServer::listen(const tcp::endpoint& endpoint)
{
    io::co_spawn(
        _executor,
        [self = shared_from_this(), endpoint]() -> io::awaitable<void> {
            co_await self->doListen(endpoint);
        },
        io::detached);
}

io::awaitable<void>
RecognitionServer::doListen(tcp::endpoint endpoint)
{
    tcp::acceptor acceptor{co_await io::this_coro::executor, endpoint};
    for (;;) {
        auto socket = co_await acceptor.async_accept(io::use_awaitable);
        if (auto id = getSessionId(socket); id) {
            RecognitionSession::create(*id, std::move(socket), _factory, _performer)->run();
        } else {
            LOGE("Unable to generate session id");
        }
    }
}

} // namespace jar
#include "intent/RecognitionServer.hpp"

#include "intent/RecognitionSession.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <jarvisto/Logger.hpp>

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
                          std::shared_ptr<WitRecognitionFactory> factory)
{
    // clang-format off
    return std::shared_ptr<RecognitionServer>(
        new RecognitionServer{std::move(executor), std::move(factory)}
    );
    // clang-format on
}

RecognitionServer::RecognitionServer(io::any_io_executor executor,
                                     std::shared_ptr<WitRecognitionFactory> factory)
    : _executor{std::move(executor)}
    , _acceptor{io::make_strand(_executor)}
    , _factory{std::move(factory)}
    , _shutdownReady{false}
    , _acceptorReady{false}
{
}

bool
RecognitionServer::listen(io::ip::port_type port)
{
    return listen(tcp::endpoint{tcp::v4(), port});
}

bool
RecognitionServer::listen(const tcp::endpoint& endpoint)
{
    if (not doListen(endpoint)) {
        LOGE("Unable to start listening");
        return false;
    }
    doAccept();
    return true;
}

bool
RecognitionServer::doListen(const tcp::endpoint& endpoint)
{
    _shutdownReady = false;

    sys::error_code ec;
    if (_acceptor.open(endpoint.protocol(), ec); ec) {
        LOGE("Unable to open acceptor: <{}>", ec.message());
        return false;
    }
    if (_acceptor.set_option(io::socket_base::reuse_address(true), ec); ec) {
        LOGE("Unable to set acceptor option: <{}>", ec.message());
        return false;
    }
    if (_acceptor.bind(endpoint, ec); ec) {
        LOGE("Unable to bind acceptor: <{}>", ec.message());
        return false;
    }
    if (_acceptor.listen(io::socket_base::max_listen_connections, ec); ec) {
        LOGE("Unable to start listening: <{}>", ec.message());
        return false;
    }

    return true;
}

void
RecognitionServer::shutdown()
{
    io::dispatch(io::bind_executor(_acceptor.get_executor(), [self = shared_from_this()]() {
        self->close();
        if (self->readyToShutdown()) {
            LOGD("Server is ready to shutdown");
            self->notifyShutdownReady();
        }
    }));

    waitForShutdown();
}

void
RecognitionServer::doAccept()
{
    _acceptor.async_accept(
        io::make_strand(_executor),
        beast::bind_front_handler(&RecognitionServer::onAcceptDone, shared_from_this()));
}

void
RecognitionServer::onAcceptDone(sys::error_code ec, tcp::socket socket)
{
    if (ec == sys::errc::operation_canceled) {
        LOGD("Accepting connection is canceled");
        return;
    }

    if (ec) {
        LOGE("Unable to accept connection: {}", ec.message());
        return;
    }

    if (not spawnSession(std::move(socket))) {
        LOGE("Unable to spawn session");
    }

    doAccept();
}

bool
RecognitionServer::spawnSession(tcp::socket socket)
{
    auto idOpt = getSessionId(socket);
    if (not idOpt) {
        LOGE("Unable to generate session id");
        return false;
    }
    const auto id = *idOpt;

    LOGD("Create new session: {}", id);
    auto session = RecognitionSession::create(id, std::move(socket), _factory);
    {
        std::lock_guard lock{_sessionsGuard};
        if (_sessions.contains(id)) {
            LOGE("Session with <{}> id already exists", id);
            return false;
        } else {
            _sessions.emplace(id, session);
        }
    }
    session->onComplete(std::bind_front(&RecognitionServer::onSessionComplete, this));
    LOGD("Run <{}> session", id);
    session->run();
    return true;
}

void
RecognitionServer::waitForShutdown()
{
    std::unique_lock lock{_shutdownGuard};
    _shutdownReadyCv.wait(lock, [this]() { return _shutdownReady; });
}

void
RecognitionServer::notifyShutdownReady()
{
    std::unique_lock lock{_shutdownGuard};
    _shutdownReady = true;
    lock.unlock();
    _shutdownReadyCv.notify_one();
}

bool
RecognitionServer::readyToShutdown() const
{
    std::scoped_lock lock{_shutdownGuard, _sessionsGuard};
    return _acceptorReady && _sessions.empty();
}

void
RecognitionServer::close()
{
    sys::error_code ec;
    _acceptor.close(ec);
    {
        std::lock_guard lock{_shutdownGuard};
        _acceptorReady = true;
    }
}

void
RecognitionServer::onSessionComplete(std::size_t id)
{
    LOGD("Session <{}> is complete", id);
    {
        std::lock_guard lock{_sessionsGuard};
        _sessions.erase(id);
    }
    if (readyToShutdown()) {
        LOGI("Server is ready to shutdown");
        notifyShutdownReady();
    }
}

} // namespace jar
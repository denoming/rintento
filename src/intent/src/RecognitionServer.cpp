#include "intent/RecognitionServer.hpp"

#include "intent/RecognitionConnection.hpp"
#include "intent/RecognitionDispatcher.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "jarvis/Logger.hpp"

namespace jar {

std::shared_ptr<RecognitionServer>
RecognitionServer::create(io::any_io_executor executor,
                          std::shared_ptr<IntentPerformer> performer,
                          std::shared_ptr<WitRecognitionFactory> factory)
{
    // clang-format off
    return std::shared_ptr<RecognitionServer>(
        new RecognitionServer{std::move(executor), std::move(performer), std::move(factory)}
    );
    // clang-format on
}

RecognitionServer::RecognitionServer(io::any_io_executor executor,
                                     std::shared_ptr<IntentPerformer> performer,
                                     std::shared_ptr<WitRecognitionFactory> factory)
    : _executor{std::move(executor)}
    , _performer{std::move(performer)}
    , _acceptor{io::make_strand(_executor)}
    , _factory{factory}
    , _shutdownReady{false}
    , _acceptorReady{false}
{
}

bool
RecognitionServer::listen(io::ip::port_type port)
{
    tcp::endpoint endpoint{io::ip::address_v4::any(), port};
    return listen(endpoint);
}

bool
RecognitionServer::listen(tcp::endpoint endpoint)
{
    _shutdownReady = false;

    sys::error_code error;
    _acceptor.open(endpoint.protocol(), error);
    if (error) {
        LOGE("Failed to open acceptor: <{}>", error.what());
        return false;
    }

    _acceptor.set_option(io::socket_base::reuse_address(true), error);
    if (error) {
        LOGE("Failed to set option: <{}>", error.what());
        return false;
    }

    _acceptor.bind(endpoint, error);
    if (error) {
        LOGE("Failed to bind: <{}>", error.what());
        return false;
    }

    _acceptor.listen(io::socket_base::max_listen_connections, error);
    if (error) {
        LOGE("Failed to listen: <{}>", error.what());
        return false;
    }

    io::dispatch(_acceptor.get_executor(),
                 beast::bind_front_handler(&RecognitionServer::accept, shared_from_this()));

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
RecognitionServer::accept()
{
    _acceptor.async_accept(
        io::make_strand(_executor),
        beast::bind_front_handler(&RecognitionServer::onAcceptDone, shared_from_this()));
}

void
RecognitionServer::onAcceptDone(sys::error_code error, tcp::socket socket)
{
    if (!error) {
        LOGD("Connection was established");
        auto connection = RecognitionConnection::create(std::move(socket));
        if (!dispatch(connection)) {
            LOGE("Failed to dispatch connection");
            connection->close();
        }
    }

    accept();
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
    std::scoped_lock lock{_shutdownGuard, _dispatchersGuard};
    return _acceptorReady && _dispatchers.empty();
}

void
RecognitionServer::close()
{
    sys::error_code error;
    _acceptor.close(error);
    {
        std::lock_guard lock{_shutdownGuard};
        _acceptorReady = true;
    }
}

bool
RecognitionServer::dispatch(std::shared_ptr<RecognitionConnection> connection)
{
    sys::error_code error;
    const auto endpoint = connection->endpointRemote(error);
    if (error) {
        LOGE("Failed to get dispatcher identity: <{}> error", error.what());
        return false;
    }

    const auto identity = endpoint.port();
    if (_dispatchers.contains(identity)) {
        LOGE("Dispatcher identity already exists");
        return false;
    }

    auto dispatcher = RecognitionDispatcher::create(identity, connection, _performer, _factory);
    {
        std::lock_guard lock{_dispatchersGuard};
        _dispatchers.emplace(identity, dispatcher);
    }
    dispatcher->onDone([this](uint16_t identity) {
        LOGD("Dispatcher <{}> has done", identity);
        {
            std::lock_guard lock{_dispatchersGuard};
            _dispatchers.erase(identity);
        }
        if (readyToShutdown()) {
            LOGD("Server is ready to shutdown");
            notifyShutdownReady();
        }
    });
    LOGD("Dispatcher <{}> is going to start", identity);
    dispatcher->dispatch();
    return true;
}

} // namespace jar
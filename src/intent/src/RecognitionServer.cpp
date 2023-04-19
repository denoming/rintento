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
        LOGE("Failed to open acceptor: <{}>", error.message());
        return false;
    }

    _acceptor.set_option(io::socket_base::reuse_address(true), error);
    if (error) {
        LOGE("Failed to set option: <{}>", error.message());
        return false;
    }

    _acceptor.bind(endpoint, error);
    if (error) {
        LOGE("Failed to bind: <{}>", error.message());
        return false;
    }

    _acceptor.listen(io::socket_base::max_listen_connections, error);
    if (error) {
        LOGE("Failed to listen: <{}>", error.message());
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
RecognitionServer::onAcceptDone(std::error_code error, tcp::socket socket)
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
    const auto endpoint = connection->endpointRemote();
    if (!endpoint) {
        LOGE("Getting dispatcher id has failed: <{}>", endpoint.error().message());
        return false;
    }

    const auto id = endpoint.value().port();
    if (_dispatchers.contains(id)) {
        LOGE("Dispatcher with <{}> id already exists", id);
        return false;
    }

    auto dispatcher = RecognitionDispatcher::create(id, connection, _performer, _factory);
    {
        std::lock_guard lock{_dispatchersGuard};
        _dispatchers.emplace(id, dispatcher);
    }
    dispatcher->onDone(std::bind_front(&RecognitionServer::onDispatchDone, this));
    LOGD("Dispatcher <{}> is going to start", id);
    dispatcher->dispatch();
    return true;
}

void
RecognitionServer::onDispatchDone(uint16_t id)
{
    LOGD("Dispatcher <{}> has done", id);
    {
        std::lock_guard lock{_dispatchersGuard};
        _dispatchers.erase(id);
    }
    if (readyToShutdown()) {
        LOGD("Server is ready to shutdown");
        notifyShutdownReady();
    }
}

} // namespace jar
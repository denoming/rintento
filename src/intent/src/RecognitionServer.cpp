#include "intent/RecognitionServer.hpp"

#include "common/Logger.hpp"
#include "intent/RecognitionConnection.hpp"

namespace jar {

RecognitionServer::RecognitionServer(net::any_io_executor& executor,
                                             IntentPerformer::Ptr performer,
                                             WitRecognitionFactory::Ptr factory)
    : _executor{executor}
    , _performer{std::move(performer)}
    , _acceptor{net::make_strand(executor)}
    , _factory{factory}
{
}

bool
RecognitionServer::listen(tcp::endpoint endpoint)
{
    sys::error_code error;
    _acceptor.open(endpoint.protocol(), error);
    if (error) {
        LOGE("Failed to open acceptor: <{}>", error.what());
        return false;
    }

    _acceptor.set_option(net::socket_base::reuse_address(true), error);
    if (error) {
        LOGE("Failed to set option: <{}>", error.what());
        return false;
    }

    _acceptor.bind(endpoint, error);
    if (error) {
        LOGE("Failed to bind: <{}>", error.what());
        return false;
    }

    _acceptor.listen(net::socket_base::max_listen_connections, error);
    if (error) {
        LOGE("Failed to listen: <{}>", error.what());
        return false;
    }

    net::dispatch(_acceptor.get_executor(),
                  beast::bind_front_handler(&RecognitionServer::accept, shared_from_this()));

    return true;
}

void
RecognitionServer::shutdown()
{
    net::dispatch(net::bind_executor(_acceptor.get_executor(),
                                     [self = shared_from_this()]() { self->close(); }));
}

void
RecognitionServer::accept()
{
    _acceptor.async_accept(
        net::make_strand(_executor),
        beast::bind_front_handler(&RecognitionServer::onAcceptDone, shared_from_this()));
}

void
RecognitionServer::onAcceptDone(sys::error_code error, tcp::socket socket)
{
    if (error) {
        LOGE("Failed to accept: <{}>", error.what());
    } else {
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
RecognitionServer::close()
{
    sys::error_code error;
    _acceptor.close(error);
}

bool
RecognitionServer::dispatch(RecognitionConnection::Ptr connection)
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
    dispatcher->whenDone([this](uint16_t identity) {
        LOGD("The <{}> dispatcher has finished", identity);
        std::lock_guard lock{_dispatchersGuard};
        _dispatchers.erase(identity);
    });
    LOGD("The <{}> dispatcher is going to start", identity);
    dispatcher->dispatch();
    return true;
}

} // namespace jar
#include "intent/IntentRecognizeServer.hpp"

#include "common/Logger.hpp"
#include "intent/IntentRecognizeConnection.hpp"

namespace jar {

IntentRecognizeServer::IntentRecognizeServer(net::any_io_executor& executor,
                                             IntentPerformer::Ptr performer,
                                             WitRecognitionFactory::Ptr factory)
    : _executor{executor}
    , _performer{std::move(performer)}
    , _acceptor{net::make_strand(executor)}
    , _factory{factory}
{
}

bool
IntentRecognizeServer::listen(tcp::endpoint endpoint)
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
                  beast::bind_front_handler(&IntentRecognizeServer::accept, shared_from_this()));

    return true;
}

void
IntentRecognizeServer::shutdown()
{
    net::dispatch(net::bind_executor(_acceptor.get_executor(),
                                     [self = shared_from_this()]() { self->close(); }));
}

void
IntentRecognizeServer::accept()
{
    _acceptor.async_accept(
        net::make_strand(_executor),
        beast::bind_front_handler(&IntentRecognizeServer::onAcceptDone, shared_from_this()));
}

void
IntentRecognizeServer::onAcceptDone(sys::error_code error, tcp::socket socket)
{
    if (error) {
        LOGE("Failed to accept: <{}>", error.what());
    } else {
        LOGD("Connection was established");
        auto connection = IntentRecognizeConnection::create(std::move(socket));
        _processor = IntentRecognizeProcessor::create(connection, _performer, _factory);
        _processor->process();
    }

    accept();
}

void
IntentRecognizeServer::close()
{
    sys::error_code error;
    _acceptor.close(error);
}

} // namespace jar
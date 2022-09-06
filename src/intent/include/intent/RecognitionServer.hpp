#pragma once

#include "intent/Constants.hpp"
#include "intent/Http.hpp"
#include "intent/RecognitionDispatcher.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

namespace jar {

class RecognitionServer : public std::enable_shared_from_this<RecognitionServer> {
public:
    using Ptr = std::shared_ptr<RecognitionServer>;

    static RecognitionServer::Ptr
    create(net::any_io_executor executor,
           IntentPerformer::Ptr performer,
           WitRecognitionFactory::Ptr factory);

    bool
    listen(net::ip::port_type port = kDefaultServerPort);

    bool
    listen(tcp::endpoint endpoint);

    void
    shutdown();

private:
    RecognitionServer(net::any_io_executor executor,
                      IntentPerformer::Ptr performer,
                      WitRecognitionFactory::Ptr factory);

    void
    accept();

    void
    onAcceptDone(sys::error_code error, tcp::socket socket);

    void
    waitForShutdown();

    void
    notifyShutdownReady();

    bool
    readyToShutdown() const;

    void
    close();

    bool
    dispatch(RecognitionConnection::Ptr connection);

private:
    net::any_io_executor _executor;
    IntentPerformer::Ptr _performer;
    WitRecognitionFactory::Ptr _factory;
    mutable std::mutex _shutdownGuard;
    std::condition_variable _shutdownReadyCv;
    bool _shutdownReady;
    bool _acceptorReady;
    tcp::acceptor _acceptor;
    mutable std::mutex _dispatchersGuard;
    std::map<std::uint16_t, RecognitionDispatcher::Ptr> _dispatchers;
};

} // namespace jar
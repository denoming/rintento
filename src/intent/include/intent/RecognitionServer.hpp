#pragma once

#include "common/Constants.hpp"
#include "intent/Http.hpp"

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

namespace jar {

class IntentPerformer;
class WitRecognitionFactory;
class RecognitionConnection;
class RecognitionDispatcher;

class RecognitionServer : public std::enable_shared_from_this<RecognitionServer> {
public:
    [[nodiscard]] static std::shared_ptr<RecognitionServer>
    create(net::any_io_executor executor,
           std::shared_ptr<IntentPerformer> performer,
           std::shared_ptr<WitRecognitionFactory> factory);

    bool
    listen(net::ip::port_type port = kDefaultServerPort);

    bool
    listen(tcp::endpoint endpoint);

    void
    shutdown();

private:
    RecognitionServer(net::any_io_executor executor,
                      std::shared_ptr<IntentPerformer> performer,
                      std::shared_ptr<WitRecognitionFactory> factory);

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
    dispatch(std::shared_ptr<RecognitionConnection> connection);

private:
    net::any_io_executor _executor;
    std::shared_ptr<IntentPerformer> _performer;
    std::shared_ptr<WitRecognitionFactory> _factory;
    mutable std::mutex _shutdownGuard;
    std::condition_variable _shutdownReadyCv;
    bool _shutdownReady;
    bool _acceptorReady;
    tcp::acceptor _acceptor;
    mutable std::mutex _dispatchersGuard;
    std::map<std::uint16_t, std::shared_ptr<RecognitionDispatcher>> _dispatchers;
};

} // namespace jar
#pragma once

#include "intent/Constants.hpp"
#include "intent/WitTypes.hpp"

#include <jarvisto/Network.hpp>

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

namespace jar {

class WitRecognitionFactory;
class RecognitionSession;
class AutomationPerformer;

class RecognitionServer : public std::enable_shared_from_this<RecognitionServer> {
public:
    [[nodiscard]] static std::shared_ptr<RecognitionServer>
    create(io::any_io_executor executor,
           std::shared_ptr<WitRecognitionFactory> factory,
           std::shared_ptr<AutomationPerformer> performer);

    bool
    listen(io::ip::port_type port);

    bool
    listen(const tcp::endpoint& endpoint);

    void
    shutdown();

private:
    RecognitionServer(io::any_io_executor executor,
                      std::shared_ptr<WitRecognitionFactory> factory,
                      std::shared_ptr<AutomationPerformer> performer);

    bool
    doListen(const tcp::endpoint& endpoint);

    void
    doAccept();

    void
    onAcceptDone(sys::error_code ec, tcp::socket socket);

    void
    runSession(tcp::socket socket);

    void
    waitForShutdown();

    void
    notifyShutdownReady();

    bool
    readyToShutdown() const;

    void
    close();

private:
    io::any_io_executor _executor;
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<AutomationPerformer> _performer;
    mutable std::mutex _shutdownGuard;
    std::condition_variable _shutdownReadyCv;
    bool _shutdownReady;
    bool _acceptorReady;
    tcp::acceptor _acceptor;
};

} // namespace jar
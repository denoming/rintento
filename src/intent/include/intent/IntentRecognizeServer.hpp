#pragma once

#include "intent/Http.hpp"
#include "intent/IntentRecognizeDispatcher.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <memory>
#include <map>
#include <mutex>

namespace jar {

class IntentRecognizeServer : public std::enable_shared_from_this<IntentRecognizeServer> {
public:
    using Ptr = std::shared_ptr<IntentRecognizeServer>;

    IntentRecognizeServer(net::any_io_executor& executor,
                          IntentPerformer::Ptr performer,
                          WitRecognitionFactory::Ptr factory);

    bool
    listen(tcp::endpoint endpoint);

    void
    shutdown();

private:
    void
    accept();

    void
    onAcceptDone(sys::error_code error, tcp::socket socket);

    void
    close();

    bool dispatch(IntentRecognizeConnection::Ptr connection);

private:
    net::any_io_executor& _executor;
    IntentPerformer::Ptr _performer;
    WitRecognitionFactory::Ptr _factory;
    tcp::acceptor _acceptor;
    std::mutex _dispatchersGuard;
    std::map<std::uint16_t, IntentRecognizeDispatcher::Ptr> _dispatchers;
};

} // namespace jar
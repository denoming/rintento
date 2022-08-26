#pragma once

#include "intent/Http.hpp"
#include "intent/RecognitionDispatcher.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <memory>
#include <map>
#include <mutex>

namespace jar {

class RecognitionServer : public std::enable_shared_from_this<RecognitionServer> {
public:
    using Ptr = std::shared_ptr<RecognitionServer>;

    RecognitionServer(net::any_io_executor& executor,
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

    bool
    dispatch(RecognitionConnection::Ptr connection);

private:
    net::any_io_executor& _executor;
    IntentPerformer::Ptr _performer;
    WitRecognitionFactory::Ptr _factory;
    tcp::acceptor _acceptor;
    std::mutex _dispatchersGuard;
    std::map<std::uint16_t, RecognitionDispatcher::Ptr> _dispatchers;
};

} // namespace jar
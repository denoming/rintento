#pragma once

#include "intent/Http.hpp"
#include "intent/IntentRecognizeProcessor.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <memory>

namespace jar {

class IntentRecognizeServer : public std::enable_shared_from_this<IntentRecognizeServer> {
public:
    using Ptr = std::shared_ptr<IntentRecognizeServer>;

    IntentRecognizeServer(net::any_io_executor& executor,
                          IntentPerformer::Ptr performer,
                          WitRecognitionFactory& factory);

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

private:
    struct ProcessorHandle {

    };

private:
    net::any_io_executor& _executor;
    IntentPerformer::Ptr _performer;
    WitRecognitionFactory& _factory;
    tcp::acceptor _acceptor;
    IntentRecognizeProcessor::Ptr _processor;
};

} // namespace jar
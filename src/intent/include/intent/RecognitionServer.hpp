#pragma once

#include <jarvisto/Cancellable.hpp>
#include <jarvisto/Network.hpp>

#include <memory>

namespace jar {

class IRecognitionFactory;
class AutomationPerformer;

class RecognitionServer : public std::enable_shared_from_this<RecognitionServer> {
public:
    using Ptr = std::shared_ptr<RecognitionServer>;

    [[nodiscard]] static Ptr
    create(io::any_io_executor executor,
           std::shared_ptr<IRecognitionFactory> factory,
           std::shared_ptr<AutomationPerformer> performer);

    void
    listen(io::ip::port_type port);

    void
    listen(const tcp::endpoint& endpoint);

private:
    RecognitionServer(io::any_io_executor executor,
                      std::shared_ptr<IRecognitionFactory> factory,
                      std::shared_ptr<AutomationPerformer> performer);

    io::awaitable<void>
    doListen(tcp::endpoint endpoint);

private:
    io::any_io_executor _executor;
    std::shared_ptr<IRecognitionFactory> _factory;
    std::shared_ptr<AutomationPerformer> _performer;
};

} // namespace jar
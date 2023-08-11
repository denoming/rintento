#include "intent/IntentSubsystem.hpp"

#include "common/Config.hpp"
#include "intent/ActionPerformer.hpp"
#include "intent/ActionRegistry.hpp"
#include "intent/RecognitionServer.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <jarvisto/Application.hpp>
#include <jarvisto/Logger.hpp>
#include <jarvisto/Worker.hpp>

#include <boost/assert.hpp>

namespace jar {

class IntentSubsystem::Impl {
public:
    Impl(std::shared_ptr<Config> config)
        : _config{config}
        , _proxyWorker{config.get()->proxyServerThreads()}
        , _recognizeWorker{config->recognizeThreads()}
    {
    }

    void
    initialize(Application& /*application*/)
    {
        _factory = std::make_unique<WitRecognitionFactory>(_config, _recognizeWorker.executor());
        _registry = std::make_unique<ActionRegistry>();
        _performer = ActionPerformer::create(*_registry);
        _server = RecognitionServer::create(_proxyWorker.executor(), _performer, _factory);
    }

    void
    setUp(Application& /*application*/)
    {
        _proxyWorker.start();
        _recognizeWorker.start();

        const auto port = _config->proxyServerPort();
        BOOST_ASSERT(_server);
        if (_server->listen(port)) {
            LOGI("Starting server on <{}> port was success", port);
        } else {
            LOGE("Starting server on <{}> port has failed", port);
        }
    }

    void
    tearDown()
    {
        if (_server) {
            _server->shutdown();
        }

        _proxyWorker.stop();
        _recognizeWorker.stop();
    }

    void
    finalize()
    {
        _server.reset();
        _performer.reset();
        _registry.reset();
        _factory.reset();
    }

private:
    Worker _proxyWorker;
    Worker _recognizeWorker;
    std::shared_ptr<Config> _config;
    std::unique_ptr<ActionRegistry> _registry;
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<ActionPerformer> _performer;
    std::shared_ptr<RecognitionServer> _server;
};

IntentSubsystem::IntentSubsystem(std::shared_ptr<Config> config)
    : _impl{std::make_unique<Impl>(std::move(config))}
{
}

IntentSubsystem::~IntentSubsystem() = default;

const char*
IntentSubsystem::name() const
{
    return "IntentSubsystem";
}

void
IntentSubsystem::initialize(Application& application)
{
    Subsystem::initialize(application);

    BOOST_ASSERT(_impl);
    _impl->initialize(application);
}

void
IntentSubsystem::setUp(Application& application)
{
    Subsystem::setUp(application);

    BOOST_ASSERT(_impl);
    _impl->setUp(application);
}

void
IntentSubsystem::tearDown()
{
    Subsystem::tearDown();

    BOOST_ASSERT(_impl);
    _impl->tearDown();
}

void
IntentSubsystem::finalize()
{
    Subsystem::finalize();

    BOOST_ASSERT(_impl);
    _impl->tearDown();
}

} // namespace jar
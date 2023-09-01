#include "intent/IntentSubsystem.hpp"

#include "intent/AutomationConfig.hpp"
#include "intent/AutomationPerformer.hpp"
#include "intent/AutomationRegistry.hpp"
#include "intent/GeneralConfig.hpp"
#include "intent/RecognitionServer.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <jarvisto/Application.hpp>
#include <jarvisto/Logger.hpp>
#include <jarvisto/Worker.hpp>

#include <boost/assert.hpp>

namespace jar {

class IntentSubsystem::Impl {
public:
    static const inline std::size_t kAutomationWorkerThreads{4};

    Impl() = default;

    void
    initialize(Application& /*application*/)
    {
        _config = std::make_unique<GeneralConfig>();
        if (not _config->load()) {
            LOGE("Unable to load general config");
        }

        _automationWorker = std::make_unique<Worker>(kAutomationWorkerThreads);
        _registry = std::make_unique<AutomationRegistry>();
        _performer = std::make_unique<AutomationPerformer>(*_registry);
        _automationConfig
            = std::make_unique<AutomationConfig>(_automationWorker->executor(), *_registry);
        if (not _automationConfig->load()) {
            LOGE("Unable to load automation config");
        }

        _recognizeWorker = std::make_unique<Worker>(_config->recognizeThreads());
        _factory = std::make_unique<WitRecognitionFactory>(_config->recognizeServerHost(),
                                                           _config->recognizeServerPort(),
                                                           _config->recognizeServerAuth());

        _proxyWorker = std::make_unique<Worker>(_config->proxyServerThreads());
        _server = RecognitionServer::create(_proxyWorker->executor(), _factory, _performer);
    }

    void
    setUp(Application& /*application*/)
    {
        BOOST_ASSERT(_automationWorker);
        _automationWorker->start();
        BOOST_ASSERT(_proxyWorker);
        _proxyWorker->start();
        BOOST_ASSERT(_recognizeWorker);
        _recognizeWorker->start();

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

        if (_proxyWorker) {
            _proxyWorker->stop();
        }
        if (_recognizeWorker) {
            _recognizeWorker->stop();
        }
        if (_automationWorker) {
            _automationWorker->stop();
        }
    }

    void
    finalize()
    {
        _server.reset();
        _factory.reset();
        _recognizeWorker.reset();
        _proxyWorker.reset();
        _automationWorker.reset();
        _performer.reset();
        _registry.reset();
        _config.reset();
        _automationConfig.reset();
    }

private:
    std::unique_ptr<GeneralConfig> _config;
    std::unique_ptr<AutomationConfig> _automationConfig;
    std::unique_ptr<AutomationRegistry> _registry;
    std::shared_ptr<AutomationPerformer> _performer;
    std::unique_ptr<Worker> _proxyWorker;
    std::unique_ptr<Worker> _recognizeWorker;
    std::unique_ptr<Worker> _automationWorker;
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<RecognitionServer> _server;
};

IntentSubsystem::IntentSubsystem()
    : _impl{std::make_unique<Impl>()}
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
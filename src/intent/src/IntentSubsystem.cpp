#include "intent/IntentSubsystem.hpp"

#include "intent/AutomationConfig.hpp"
#include "intent/AutomationPerformer.hpp"
#include "intent/AutomationRegistry.hpp"
#include "intent/GeneralConfig.hpp"
#include "intent/RecognitionServer.hpp"
#include "wit/RecognitionFactory.hpp"

#include <jarvisto/Application.hpp>
#include <jarvisto/Logger.hpp>
#include <jarvisto/Worker.hpp>

#include <boost/assert.hpp>

namespace jar {

class IntentSubsystem::Impl {
public:
    Impl() = default;

    void
    initialize(Application& /*application*/)
    {
        _registry = std::make_shared<AutomationRegistry>();

        _generalConfig = std::make_unique<GeneralConfig>();
        if (not _generalConfig->load()) {
            LOGE("Unable to load general config");
        }
        _automationConfig = std::make_unique<AutomationConfig>(_registry);
        if (not _automationConfig->load()) {
            LOGE("Unable to load automation config");
        }

        _worker = std::make_unique<Worker>(_generalConfig->serverThreads());
        _performer = AutomationPerformer::create(_worker->executor(), _registry);
        _factory
            = std::make_unique<wit::RecognitionFactory>(_generalConfig->recognitionServerHost(),
                                                        _generalConfig->recognitionServerPort(),
                                                        _generalConfig->recognitionServerAuth());
        _server = RecognitionServer::create(_worker->executor(), _factory, _performer);
    }

    void
    setUp(Application& /*application*/)
    {
        BOOST_ASSERT(_worker);
        _worker->start();

        const auto port = _generalConfig->serverPort();
        BOOST_ASSERT(_server);
        _server->listen(port);
    }

    void
    tearDown()
    {
        if (_worker) {
            _worker->stop();
        }
    }

    void
    finalize()
    {
        _server.reset();
        _factory.reset();
        _worker.reset();
        _performer.reset();
        _registry.reset();
        _generalConfig.reset();
        _automationConfig.reset();
    }

private:
    std::unique_ptr<GeneralConfig> _generalConfig;
    std::unique_ptr<AutomationConfig> _automationConfig;
    std::shared_ptr<AutomationRegistry> _registry;
    std::shared_ptr<AutomationPerformer> _performer;
    std::unique_ptr<Worker> _worker;
    std::shared_ptr<wit::RecognitionFactory> _factory;
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
    _impl->finalize();
}

} // namespace jar
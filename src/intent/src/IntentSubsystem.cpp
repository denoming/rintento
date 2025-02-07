#include "intent/IntentSubsystem.hpp"

#include "intent/AutomationPerformer.hpp"
#include "intent/AutomationRegistry.hpp"
#include "intent/Config.hpp"
#include "intent/RecognitionServer.hpp"
#include "rintento/Options.hpp"
#ifdef ENABLE_WIT_SUPPORT
#include "wit/RecognitionFactory.hpp"
#endif

#include <jarvisto/core/Application.hpp>
#include <jarvisto/core/Logger.hpp>
#include <jarvisto/network/Worker.hpp>

#include <boost/assert.hpp>

namespace jar {

namespace {

std::shared_ptr<IRecognitionFactory>
getFactory()
{
#ifdef ENABLE_WIT_SUPPORT
    return std::make_shared<wit::RecognitionFactory>();
#else
    LOGE("There is no recognition provider");
    return {};
#endif
}

} // namespace

class IntentSubsystem::Impl {
public:
    Impl() = default;

    void
    initialize(Application& /*application*/)
    {
        _registry = std::make_shared<AutomationRegistry>();

        _config = std::make_unique<Config>(_registry);
        if (not _config->load()) {
            LOGE("Unable to load config");
        }

        _worker = std::make_unique<Worker>(_config->serverThreads());
        _performer = AutomationPerformer::create(_worker->executor(), _registry);
        _factory = getFactory();
        if (not _factory) {
            LOGE("Recognition factory is not available");
        } else {
            _server = RecognitionServer::create(_worker->executor(), _factory, _performer);
        }
    }

    void
    setUp(Application& /*application*/)
    {
        BOOST_ASSERT(_worker);
        _worker->start();

        const auto port = _config->serverPort();
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
        _config.reset();
        _registry.reset();
    }

private:
    std::unique_ptr<Config> _config;
    std::shared_ptr<AutomationRegistry> _registry;
    std::shared_ptr<AutomationPerformer> _performer;
    std::unique_ptr<Worker> _worker;
    std::shared_ptr<IRecognitionFactory> _factory;
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
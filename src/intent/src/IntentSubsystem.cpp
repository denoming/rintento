#include "intent/IntentSubsystem.hpp"

#include "common/Config.hpp"
#include "intent/Constants.hpp"
#include "intent/IntentPerformer.hpp"
#include "intent/RecognitionServer.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "jarvis/Application.hpp"
#include "jarvis/Logger.hpp"
#include "jarvis/Worker.hpp"

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
        _performer = IntentPerformer::create();
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
        _factory.reset();
    }

private:
    Worker _proxyWorker;
    Worker _recognizeWorker;
    Config::Ptr _config;
    WitRecognitionFactory::Ptr _factory;
    IntentPerformer::Ptr _performer;
    RecognitionServer::Ptr _server;
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
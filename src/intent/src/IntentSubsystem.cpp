#include "intent/IntentSubsystem.hpp"

#include "intent/AutomationConfig.hpp"
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
    Impl() = default;

    void
    initialize(Application& /*application*/)
    {
        if (not _config.load()) {
            LOGE("Unable to load general config");
        }

        _proxyWorker = std::make_unique<Worker>(_config.proxyServerThreads());
        _recognizeWorker = std::make_unique<Worker>(_config.recognizeThreads());
        _factory = std::make_unique<WitRecognitionFactory>(_config.recognizeServerHost(),
                                                           _config.recognizeServerPort(),
                                                           _config.recognizeServerAuth(),
                                                           _recognizeWorker->executor());
        _server = RecognitionServer::create(_proxyWorker->executor(), _factory);
    }

    void
    setUp(Application& /*application*/)
    {
        BOOST_ASSERT(_proxyWorker);
        _proxyWorker->start();
        BOOST_ASSERT(_recognizeWorker);
        _recognizeWorker->start();

        const auto port = _config.proxyServerPort();
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
    }

    void
    finalize()
    {
        _server.reset();
        _factory.reset();
        _recognizeWorker.reset();
        _proxyWorker.reset();
    }

private:
    GeneralConfig _config;

    std::unique_ptr<Worker> _proxyWorker;
    std::unique_ptr<Worker> _recognizeWorker;
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
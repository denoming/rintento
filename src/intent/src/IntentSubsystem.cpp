#include "intent/IntentSubsystem.hpp"

#include "common/Config.hpp"
#include "intent/Constants.hpp"
#include "intent/IntentPerformer.hpp"
#include "intent/RecognitionServer.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "jarvis/Application.hpp"
#include "jarvis/Logger.hpp"

namespace jar {

IntentSubsystem::IntentSubsystem(std::shared_ptr<Config> config)
    : _config{config}
{
}

const char*
IntentSubsystem::name() const
{
    return "IntentSubsystem";
}

void
IntentSubsystem::initialize(Application& application)
{
    Subsystem::initialize(application);

    _factory = std::make_shared<WitRecognitionFactory>(_config, _recognizeWorker.executor());
    _performer = IntentPerformer::create();
    _server = RecognitionServer::create(_proxyWorker.executor(), _performer, _factory);
}

void
IntentSubsystem::setUp(Application& application)
{
    Subsystem::setUp(application);

    _proxyWorker.start(_config->proxyServerThreads());
    _recognizeWorker.start(_config->recognizeThreads());

    const auto port = _config->proxyServerPort();
    assert(_server);
    if (_server->listen(port)) {
        LOGI("Starting server on <{}> port was success", port);
    } else {
        LOGE("Starting server on <{}> port has failed", port);
    }
}

void
IntentSubsystem::tearDown()
{
    Subsystem::tearDown();

    if (_server) {
        _server->shutdown();
    }

    _proxyWorker.stop();
    _recognizeWorker.stop();
}

void
IntentSubsystem::finalize()
{
    Subsystem::finalize();

    _server.reset();
    _performer.reset();
    _factory.reset();
}

} // namespace jar
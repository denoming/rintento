#include "intent/IntentSubsystem.hpp"

#include "common/Application.hpp"
#include "common/Logger.hpp"
#include "intent/Constants.hpp"
#include "intent/IntentPerformer.hpp"
#include "intent/RecognitionServer.hpp"
#include "intent/WitRecognitionFactory.hpp"

namespace jar {

const char*
IntentSubsystem::name() const
{
    return "IntentSubsystem";
}

void
IntentSubsystem::initialize(Application& application)
{
    Subsystem::initialize(application);

    _factory = std::make_shared<WitRecognitionFactory>(_worker.executor());
    _performer = IntentPerformer::create();
    _server = RecognitionServer::create(_worker.executor(), _performer, _factory);
}

void
IntentSubsystem::setUp(Application& application)
{
    Subsystem::setUp(application);

    _worker.start();

    auto port = kDefaultServerPort;
    if (application.options().contains("port")) {
        port = application.options().at("port").as<std::uint16_t>();
    }

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

    _worker.stop();
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
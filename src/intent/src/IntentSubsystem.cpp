#include "intent/IntentSubsystem.hpp"

#include "common/Logger.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "intent/IntentPerformer.hpp"
#include "intent/RecognitionServer.hpp"

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

    assert(_server);
    const tcp::endpoint ServerEndpoint{net::ip::make_address("0.0.0.0"), 8080};
    _server->listen(ServerEndpoint);
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
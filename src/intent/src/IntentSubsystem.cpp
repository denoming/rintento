#include "intent/IntentSubsystem.hpp"

#include "common/Config.hpp"
#include "intent/ActionPerformer.hpp"
#include "intent/ActionRegistry.hpp"
#include "intent/PositioningClient.hpp"
#include "intent/RecognitionServer.hpp"
#include "intent/WitRecognitionFactory.hpp"
#include "intent/registry/GetRainyStatusAction.hpp"
#include "jarvis/Application.hpp"
#include "jarvis/Logger.hpp"
#include "jarvis/Worker.hpp"
#include "jarvis/speaker/SpeakerClient.hpp"
#include "jarvis/weather/WeatherClient.hpp"

#include <boost/assert.hpp>

namespace jar {

namespace {

std::unique_ptr<SpeakerClient>
getSpeakerClient()
{
    try {
        return std::make_unique<SpeakerClient>();
    } catch (const std::exception& e) {
        LOGE("Error on create speaker client: {}", e.what());
        return {};
    }
}

std::unique_ptr<WeatherClient>
getWeatherClient()
{
    try {
        return std::make_unique<WeatherClient>();
    } catch (const std::exception& e) {
        LOGE("Error on create weather client: {}", e.what());
        return {};
    }
}

std::unique_ptr<PositioningClient>
getPositioningClient()
{
    try {
        return std::make_unique<PositioningClient>();
    } catch (const std::exception& e) {
        LOGE("Error on create positioning client: {}", e.what());
        return {};
    }
}

} // namespace

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

        _positioningClient = getPositioningClient();
        _speakerClient = getSpeakerClient();
        _weatherClient = getWeatherClient();
    }

    void
    setUp(Application& /*application*/)
    {
        _proxyWorker.start();
        _recognizeWorker.start();

        registerIntents();

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
        _speakerClient.reset();
        _weatherClient.reset();
        _positioningClient.reset();
        _server.reset();
        _performer.reset();
        _registry.reset();
        _factory.reset();
    }

private:
    [[nodiscard]] bool
    hasSpeakerService() const
    {
        return bool(_speakerClient);
    }

    [[nodiscard]] bool
    hasWeatherService() const
    {
        return bool(_weatherClient);
    }

    [[nodiscard]] bool
    hasPositioningService() const
    {
        return bool(_positioningClient);
    }

    void
    registerIntents()
    {
        if (!hasWeatherService()) {
            LOGE("Weather service is not available");
            return;
        }
        if (!hasSpeakerService()) {
            LOGE("Speaker service is not available");
            return;
        }
        if (!hasPositioningService()) {
            LOGE("Positioning service is not available");
            return;
        }

        _registry->add(GetRainyStatusAction::create(
            "get_rainy_status", *_positioningClient, *_speakerClient, *_weatherClient));
    }

private:
    Worker _proxyWorker;
    Worker _recognizeWorker;
    std::shared_ptr<Config> _config;
    std::unique_ptr<ActionRegistry> _registry;
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<ActionPerformer> _performer;
    std::shared_ptr<RecognitionServer> _server;
    std::unique_ptr<PositioningClient> _positioningClient;
    std::unique_ptr<SpeakerClient> _speakerClient;
    std::unique_ptr<WeatherClient> _weatherClient;
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
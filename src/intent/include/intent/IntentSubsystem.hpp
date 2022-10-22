#pragma once

#include "jarvis/Subsystem.hpp"
#include "common/Worker.hpp"

namespace jar {

class Config;
class WitRecognitionFactory;
class IntentPerformer;
class RecognitionServer;

class IntentSubsystem final : public Subsystem {
public:
    IntentSubsystem(std::shared_ptr<Config> config);

    const char*
    name() const final;

    void
    initialize(Application& application) final;

    void
    setUp(Application& application) final;

    void
    tearDown() final;

    void
    finalize() final;

private:
    Worker _proxyWorker;
    Worker _recognizeWorker;
    std::shared_ptr<Config> _config;
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<IntentPerformer> _performer;
    std::shared_ptr<RecognitionServer> _server;
};

} // namespace jar
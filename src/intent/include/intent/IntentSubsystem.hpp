#pragma once

#include "common/Subsystem.hpp"
#include "common/Worker.hpp"

namespace jar {

class WitRecognitionFactory;
class IntentPerformer;
class RecognitionServer;

class IntentSubsystem final : public Subsystem {
public:
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
    Worker _worker;
    std::shared_ptr<WitRecognitionFactory> _factory;
    std::shared_ptr<IntentPerformer> _performer;
    std::shared_ptr<RecognitionServer> _server;
};

} // namespace jar
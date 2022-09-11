#pragma once

#include "intent/Http.hpp"

#include <memory>

namespace jar {

class Config;
class WitMessageRecognition;
class WitSpeechRecognition;

class WitRecognitionFactory {
public:
    using Ptr = std::shared_ptr<WitRecognitionFactory>;

    WitRecognitionFactory(std::shared_ptr<Config> config, net::any_io_executor executor);

    std::shared_ptr<WitMessageRecognition>
    message();

    std::shared_ptr<WitSpeechRecognition>
    speech();

private:
    std::shared_ptr<Config> _config;
    ssl::context _context;
    net::any_io_executor _executor;
};

} // namespace jar
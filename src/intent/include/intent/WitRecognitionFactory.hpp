#pragma once

#include "jarvis/Network.hpp"

#include <memory>

namespace jar {

class Config;
class WitMessageRecognition;
class WitSpeechRecognition;

class WitRecognitionFactory {
public:
    using Ptr = std::shared_ptr<WitRecognitionFactory>;

    WitRecognitionFactory(std::shared_ptr<Config> config, io::any_io_executor executor);

    std::shared_ptr<WitMessageRecognition>
    message();

    std::shared_ptr<WitSpeechRecognition>
    speech();

private:
    std::shared_ptr<Config> _config;
    ssl::context _context;
    io::any_io_executor _executor;
};

} // namespace jar
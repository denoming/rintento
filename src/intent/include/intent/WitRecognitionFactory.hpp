#pragma once

#include <jarvisto/Network.hpp>

#include <memory>

namespace jar {

class GeneralConfig;
class WitMessageRecognition;
class WitSpeechRecognition;

class WitRecognitionFactory {
public:
    WitRecognitionFactory(std::string host,
                          std::string port,
                          std::string auth,
                          io::any_io_executor executor);

    std::shared_ptr<WitMessageRecognition>
    message();

    std::shared_ptr<WitSpeechRecognition>
    speech();

private:
    std::string _host;
    std::string _port;
    std::string _auth;
    io::any_io_executor _executor;
    ssl::context _context;
};

} // namespace jar
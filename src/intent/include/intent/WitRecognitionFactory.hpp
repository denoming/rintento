#pragma once

#include "intent/WitMessageRecognition.hpp"
#include "intent/WitSpeechRecognition.hpp"

#include <jarvisto/Network.hpp>
#include <jarvisto/SecureContext.hpp>

#include <memory>

namespace jar {

class WitRecognitionFactory {
public:
    WitRecognitionFactory(std::string host, std::string port, std::string auth);

    std::shared_ptr<WitMessageRecognition>
    message(io::any_io_executor executor, std::shared_ptr<WitMessageRecognition::Channel> channel);

    std::shared_ptr<WitSpeechRecognition>
    speech(io::any_io_executor executor, std::shared_ptr<WitSpeechRecognition::Channel> channel);

private:
    std::string _host;
    std::string _port;
    std::string _auth;
    SecureContext _context;
};

} // namespace jar
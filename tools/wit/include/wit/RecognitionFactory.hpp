#pragma once

#include "wit/MessageRecognition.hpp"
#include "wit/SpeechRecognition.hpp"

#include <jarvisto/Network.hpp>
#include <jarvisto/SecureContext.hpp>

#include <memory>

namespace jar::wit {

class RecognitionFactory {
public:
    RecognitionFactory(std::string host, std::string port, std::string auth);

    std::shared_ptr<MessageRecognition>
    message(io::any_io_executor executor, std::shared_ptr<MessageRecognition::Channel> channel);

    std::shared_ptr<SpeechRecognition>
    speech(io::any_io_executor executor, std::shared_ptr<SpeechRecognition::Channel> channel);

private:
    std::string _host;
    std::string _port;
    std::string _auth;
    SecureContext _context;
};

} // namespace jar::wit
#include "wit/RecognitionFactory.hpp"

namespace jar::wit {

RecognitionFactory::RecognitionFactory(std::string host, std::string port, std::string auth)
    : _host{std::move(host)}
    , _port{std::move(port)}
    , _auth{std::move(auth)}
{
}

std::shared_ptr<MessageRecognition>
RecognitionFactory::message(io::any_io_executor executor,
                            std::shared_ptr<MessageRecognition::Channel> channel)
{
    return MessageRecognition::create(
        std::move(executor), _context.ref(), _host, _port, _auth, std::move(channel));
}

std::shared_ptr<SpeechRecognition>
RecognitionFactory::speech(io::any_io_executor executor,
                           std::shared_ptr<SpeechRecognition::Channel> channel)
{
    return SpeechRecognition::create(
        std::move(executor), _context.ref(), _host, _port, _auth, std::move(channel));
}

} // namespace jar::wit
#include "intent/RecognitionMessageHandler.hpp"

#include "intent/RecognitionConnection.hpp"
#include "intent/Utils.hpp"
#include "intent/WitMessageRecognition.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <jarvis/Logger.hpp>

#include <boost/assert.hpp>

namespace jar {

std::shared_ptr<RecognitionMessageHandler>
RecognitionMessageHandler::create(std::shared_ptr<RecognitionConnection> connection,
                                  std::shared_ptr<WitRecognitionFactory> factory)
{
    // clang-format off
    return std::shared_ptr<RecognitionMessageHandler>(
        new RecognitionMessageHandler(std::move(connection), std::move(factory))
    );
    // clang-format on
}

RecognitionMessageHandler::RecognitionMessageHandler(
    std::shared_ptr<RecognitionConnection> connection,
    std::shared_ptr<WitRecognitionFactory> factory)
    : RecognitionHandler{std::move(connection)}
    , _factory{std::move(factory)}
{
    BOOST_ASSERT(_factory);
}

void
RecognitionMessageHandler::handle(Buffer& buffer, Parser& parser)
{
    if (!canHandle(parser.get())) {
        RecognitionHandler::handle(buffer, parser);
        return;
    }

    const auto request = parser.release();
    if (auto messageOpt = parser::peekMessage(request.target()); messageOpt) {
        _message = std::move(*messageOpt);
    } else {
        LOGE("Missing message in request target");
        onRecognitionError(std::make_error_code(std::errc::bad_message));
        return;
    }

    _recognition = createRecognition();
    BOOST_ASSERT(_recognition);
    _recognition->run();
}

bool
RecognitionMessageHandler::canHandle(const Parser::value_type& request) const
{
    return parser::isMessageTarget(request.target());
}

std::shared_ptr<WitMessageRecognition>
RecognitionMessageHandler::createRecognition()
{
    auto recognition = _factory->message();
    BOOST_ASSERT(recognition);
    recognition->onDone(
        [weakSelf = weak_from_this(), executor = connection().executor()](auto result, auto error) {
            if (error) {
                io::post(executor, [weakSelf, error]() {
                    if (auto self = weakSelf.lock()) {
                        self->onRecognitionError(error);
                    }
                });
            } else {
                io::post(executor, [weakSelf, result = std::move(result)]() {
                    if (auto self = weakSelf.lock()) {
                        self->onRecognitionSuccess(std::move(result));
                    }
                });
            }
        });
    recognition->onData([weakSelf = weak_from_this()]() {
        if (auto self = weakSelf.lock()) {
            self->onRecognitionData();
        }
    });
    return recognition;
}

void
RecognitionMessageHandler::onRecognitionData()
{
    LOGD("Feed up the recognition using given message: <{}> length", _message.size());
    BOOST_ASSERT(_recognition);
    _recognition->feed(std::move(_message));
}

void
RecognitionMessageHandler::onRecognitionError(std::error_code error)
{
    LOGD("Submit recognition error: <{}>", error.message());
    sendResponse(error);
    submit(error);
}

void
RecognitionMessageHandler::onRecognitionSuccess(Utterances result)
{
    LOGD("Submit recognition success: <{}> size", result.size());
    sendResponse(result);
    submit(std::move(result));
}

} // namespace jar
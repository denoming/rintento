#include "intent/IntentRecognizeProcessor.hpp"

#include "common/Logger.hpp"
#include "intent/IntentRecognizeMessage.hpp"
#include "intent/IntentRecognizeSpeech.hpp"
#include "intent/IntentRecognizeMessageHandler.hpp"
#include "intent/IntentRecognizeSpeechHandler.hpp"
#include "intent/IntentRecognizeTerminalHandler.hpp"
#include "intent/Utils.hpp"

namespace jar {

IntentRecognizeProcessor::IntentRecognizeProcessor(IntentRecognizeConnection::Ptr connection,
                                                   IntentPerformer::Ptr performer,
                                                   WitRecognitionFactory::Ptr factory)
    : _connection{std::move(connection)}
    , _performer{std::move(performer)}
    , _factory{std::move(factory)}
{
}

IntentRecognizeProcessor::Ptr
IntentRecognizeProcessor::create(IntentRecognizeConnection::Ptr connection,
                                 IntentPerformer::Ptr performer,
                                 WitRecognitionFactory::Ptr factory)
{
    assert(connection);
    assert(performer);
    assert(factory);

    // clang-format off
    return std::shared_ptr<IntentRecognizeProcessor>(
        new IntentRecognizeProcessor{std::move(connection), std::move(performer), std::move(factory)}
    );
    // clang-format on
}

void
IntentRecognizeProcessor::process()
{
    readHeader();
}

void
IntentRecognizeProcessor::readHeader()
{
    _connection->readHeader<http::empty_body>(
        [self = shared_from_this()](auto& buffer, auto& parser, auto error) {
            self->onReadHeaderDone(buffer, parser, error);
        });
}

void
IntentRecognizeProcessor::onReadHeaderDone(beast::flat_buffer& buffer,
                                           http::request_parser<http::empty_body>& parser,
                                           sys::error_code error)
{
    if (error == http::error::end_of_stream) {
        LOGI("Connection was closed");
        return;
    }
    if (error) {
        LOGE("Failed to read: <{}>", error.what());
        return;
    }

    _handler = getHandler();
    assert(_handler);
    _handler->handle(buffer, parser);
}

void
IntentRecognizeProcessor::onComplete(Utterances utterances, sys::error_code error)
{
    if (error) {
        LOGE("Recognize request has failed");
    } else {
        LOGD("Recognize request was successful: <{}> size", utterances.size());
        _performer->perform(std::move(utterances));
    }

    readHeader();
}

IntentRecognizeHandler::Ptr
IntentRecognizeProcessor::getHandler()
{
    auto handler1 = std::make_unique<IntentRecognizeMessageHandler>(
        _connection, _factory, [this](auto result, sys::error_code error) {
            LOGD("Recognize message handler was done: failed<{}>", error.failed());
            onComplete(std::move(result), error);
        });
    auto handler2 = std::make_unique<IntentRecognizeSpeechHandler>(
        _connection, _factory, [this](auto result, auto error) {
            LOGD("Recognize speech handler was done: failed<{}>", error.failed());
            onComplete(std::move(result), error);
        });
    auto handler3 = std::make_unique<IntentRecognizeTerminalHandler>(
        _connection, [this](auto result, auto error) {
            LOGD("Terminal handler was done");
            onComplete(std::move(result), error);
        });
    handler2->setNext(std::move(handler3));
    handler1->setNext(std::move(handler2));
    return handler1;
}

} // namespace jar
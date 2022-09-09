#include "intent/RecognitionDispatcher.hpp"

#include "common/Logger.hpp"
#include "intent/RecognitionMessageHandler.hpp"
#include "intent/RecognitionSpeechHandler.hpp"
#include "intent/RecognitionTerminalHandler.hpp"
#include "intent/Utils.hpp"

namespace jar {

RecognitionDispatcher::Ptr
RecognitionDispatcher::create(uint16_t identity,
                              RecognitionConnection::Ptr connection,
                              IntentPerformer::Ptr performer,
                              WitRecognitionFactory::Ptr factory)
{
    // clang-format off
    return std::shared_ptr<RecognitionDispatcher>(
        new RecognitionDispatcher{identity, std::move(connection), std::move(performer), std::move(factory)}
    );
    // clang-format on
}

RecognitionDispatcher::RecognitionDispatcher(uint16_t identity,
                                             RecognitionConnection::Ptr connection,
                                             IntentPerformer::Ptr performer,
                                             WitRecognitionFactory::Ptr factory)
    : _identity{identity}
    , _connection{std::move(connection)}
    , _performer{std::move(performer)}
    , _factory{std::move(factory)}
{
    assert(identity > 0);
    assert(connection);
    assert(performer);
    assert(factory);
}

uint16_t
RecognitionDispatcher::identity() const
{
    return _identity;
}

void
RecognitionDispatcher::dispatch()
{
    readHeader();
}

void
RecognitionDispatcher::readHeader()
{
    _connection->readHeader<http::empty_body>(
        [weakSelf = weak_from_this()](auto& buffer, auto& parser, auto error) {
            if (auto self = weakSelf.lock()) {
                self->onReadHeaderDone(buffer, parser, error);
            }
        });
}

void
RecognitionDispatcher::onReadHeaderDone(beast::flat_buffer& buffer,
                                        http::request_parser<http::empty_body>& parser,
                                        sys::error_code error)
{
    if (error == http::error::end_of_stream) {
        LOGI("Connection was closed");
        finalize();
        return;
    }
    if (error) {
        LOGE("Failed to read: <{}>", error.what());
        finalize();
        return;
    }

    _handler = getHandler();
    assert(_handler);
    _handler->handle(buffer, parser);
}

void
RecognitionDispatcher::onComplete(Utterances utterances, sys::error_code error)
{
    if (error) {
        LOGE("Request dispatching has failed: <{}> error", error.message());
    } else {
        LOGD("Request dispatching has succeed: <{}> size", utterances.size());
        _performer->perform(std::move(utterances));
    }

    readHeader();
}

RecognitionHandler::Ptr
RecognitionDispatcher::getHandler()
{
    auto handler1 = RecognitionMessageHandler::create(
        _connection, _factory, [weakSelf = weak_from_this()](auto result, auto error) {
            if (auto self = weakSelf.lock()) {
                LOGD("Recognize message handler has finished: success<{}>", !error.failed());
                self->onComplete(std::move(result), error);
            }
        });
    auto handler2 = RecognitionSpeechHandler::create(
        _connection, _factory, [weakSelf = weak_from_this()](auto result, auto error) {
            if (auto self = weakSelf.lock()) {
                LOGD("Recognize speech handler has finished: success<{}>", !error.failed());
                self->onComplete(std::move(result), error);
            }
        });
    auto handler3 = RecognitionTerminalHandler::create(
        _connection, [weakSelf = weak_from_this()](auto result, auto error) {
            if (auto self = weakSelf.lock()) {
                LOGD("Terminal handler has finished");
                self->onComplete(std::move(result), error);
            }
        });
    handler2->setNext(std::move(handler3));
    handler1->setNext(std::move(handler2));
    return handler1;
}

void
RecognitionDispatcher::finalize()
{
    if (_doneCallback) {
        _doneCallback(_identity);
    }
}

} // namespace jar
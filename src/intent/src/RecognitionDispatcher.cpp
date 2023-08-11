#include "intent/RecognitionDispatcher.hpp"

#include "intent/ActionPerformer.hpp"
#include "intent/RecognitionConnection.hpp"
#include "intent/RecognitionMessageHandler.hpp"
#include "intent/RecognitionSpeechHandler.hpp"
#include "intent/RecognitionTerminalHandler.hpp"
#include "intent/Utils.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>

namespace jar {

std::shared_ptr<RecognitionDispatcher>
RecognitionDispatcher::create(uint16_t identity,
                              std::shared_ptr<RecognitionConnection> connection,
                              std::shared_ptr<ActionPerformer> performer,
                              std::shared_ptr<WitRecognitionFactory> factory)
{
    // clang-format off
    return std::shared_ptr<RecognitionDispatcher>(
        new RecognitionDispatcher{identity, std::move(connection), std::move(performer), std::move(factory)}
    );
    // clang-format on
}

RecognitionDispatcher::RecognitionDispatcher(uint16_t id,
                                             std::shared_ptr<RecognitionConnection> connection,
                                             std::shared_ptr<ActionPerformer> performer,
                                             std::shared_ptr<WitRecognitionFactory> factory)
    : _id{id}
    , _connection{std::move(connection)}
    , _performer{std::move(performer)}
    , _factory{std::move(factory)}
{
    BOOST_ASSERT(_id);
    BOOST_ASSERT(_connection);
    BOOST_ASSERT(_performer);
    BOOST_ASSERT(_factory);
}

void
RecognitionDispatcher::onDone(std::move_only_function<OnDone> callback)
{
    _onDone = std::move(callback);
}

uint16_t
RecognitionDispatcher::id() const
{
    return _id;
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
                                        std::error_code error)
{
    if (error == sys::error_code{http::error::end_of_stream}) {
        LOGI("Connection if <{}> dispatcher was closed", _id);
        finalize();
        return;
    }
    if (error) {
        LOGE("Failed to read with <{}> error", error.message());
        finalize();
        return;
    }

    _handler = getHandler();
    BOOST_ASSERT(_handler);
    _handler->handle(buffer, parser);
}

void
RecognitionDispatcher::onDone(wit::Utterances utterances, std::error_code error)
{
    if (error) {
        LOGE("The <{}> dispatcher has failed: <{}> error", _id, error.message());
    } else {
        LOGD("The <{}> dispatcher has succeed: <{}> size", _id, utterances.size());
        _performer->perform(std::move(utterances));
    }

    readHeader();
}

std::shared_ptr<RecognitionHandler>
RecognitionDispatcher::getHandler()
{
    auto handler1 = RecognitionMessageHandler::create(_connection, _factory);
    handler1->onDone(
        [weakSelf = weak_from_this(), id = _id](wit::Utterances result, std::error_code error) {
            if (auto self = weakSelf.lock()) {
                LOGD("Message handler of <{}> dispatcher has done: success<{}>", id, !error);
                self->onDone(std::move(result), error);
            }
        });
    auto handler2 = RecognitionSpeechHandler::create(_connection, _factory);
    handler2->onDone(
        [weakSelf = weak_from_this(), id = _id](wit::Utterances result, std::error_code error) {
            if (auto self = weakSelf.lock()) {
                LOGD("Speech handler of <{}> dispatcher has done: success<{}>", id, !error);
                self->onDone(std::move(result), error);
            }
        });
    auto handler3 = RecognitionTerminalHandler::create(_connection);
    handler3->onDone(
        [weakSelf = weak_from_this(), id = _id](wit::Utterances result, std::error_code error) {
            if (auto self = weakSelf.lock()) {
                LOGD("Terminal handler of <{}> dispatcher has done", id);
                self->onDone(std::move(result), error);
            }
        });
    handler2->setNext(std::move(handler3));
    handler1->setNext(std::move(handler2));
    return handler1;
}

void
RecognitionDispatcher::finalize()
{
    if (_onDone) {
        _onDone(_id);
    }
}

} // namespace jar
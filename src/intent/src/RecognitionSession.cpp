#include "intent/RecognitionSession.hpp"

#include "intent/Formatters.hpp"
#include "intent/RecognitionMessageHandler.hpp"
#include "intent/RecognitionSpeechHandler.hpp"
#include "intent/RecognitionTerminalHandler.hpp"
#include "intent/Utils.hpp"
#include "intent/WitRecognitionFactory.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>

namespace jar {

std::shared_ptr<RecognitionSession>
RecognitionSession::create(std::size_t id,
                           tcp::socket&& socket,
                           std::shared_ptr<WitRecognitionFactory> factory)
{
    // clang-format off
    return std::shared_ptr<RecognitionSession>(
        new RecognitionSession{id, std::move(socket), std::move(factory)}
    );
    // clang-format on
}

RecognitionSession::RecognitionSession(std::size_t id,
                                       tcp::socket&& socket,
                                       std::shared_ptr<WitRecognitionFactory> factory)
    : _id{id}
    , _stream{std::move(socket)}
    , _factory{std::move(factory)}
{
    BOOST_ASSERT(_id);
    BOOST_ASSERT(_factory);
}

void
RecognitionSession::onComplete(std::move_only_function<OnComplete> callback)
{
    _onComplete = std::move(callback);
}

std::size_t
RecognitionSession::id() const
{
    return _id;
}

void
RecognitionSession::run()
{
    doReadHeader();
}

void
RecognitionSession::doReadHeader()
{
    http::async_read_header(
        _stream,
        _buffer,
        _parser,
        beast::bind_front_handler(&RecognitionSession::onReadHeaderDone, shared_from_this()));
}

void
RecognitionSession::onReadHeaderDone(sys::error_code ec, std::size_t bytes)
{
    LOGD("Reading header is complete: id<{}>, ec<{}>, bytes<{}>", _id, ec.message(), bytes);

    if (bytes == 0 or ec == http::error::end_of_stream) {
        LOGD("The end of stream of <{}> session", _id);
        finalize();
        return;
    }

    if (ec) {
        LOGE("Unable to read from <{}> session: error<{}>", _id, ec.message());
        finalize();
        return;
    }

    LOGD("Call handler of <{}> session", _id);
    _handler = getHandler();
    BOOST_ASSERT(_handler);
    _handler->handle();
}

void
RecognitionSession::onComplete(wit::Utterances utterances, std::error_code ec)
{
    if (ec) {
        LOGE("The <{}> session has failed: error<{}>", _id, ec.message());
    } else {
        LOGI("The <{}> session has succeed: size<{}>", _id, utterances.size());
    }

    doReadHeader();
}

std::shared_ptr<RecognitionHandler>
RecognitionSession::getHandler()
{
    auto handler1 = RecognitionMessageHandler::create(_stream, _buffer, _parser, _factory);
    handler1->onComplete(
        [weakSelf = weak_from_this(), id = _id](wit::Utterances result, std::error_code ec) {
            if (auto self = weakSelf.lock()) {
                LOGD("Message handler of <{}> session is complete: success<{}>", id, !ec);
                self->onComplete(std::move(result), ec);
            }
        });
    auto handler2 = RecognitionSpeechHandler::create(_stream, _buffer, _parser, _factory);
    handler2->onComplete(
        [weakSelf = weak_from_this(), id = _id](wit::Utterances result, std::error_code ec) {
            if (auto self = weakSelf.lock()) {
                LOGD("Speech handler of <{}> session is complete: success<{}>", id, !ec);
                self->onComplete(std::move(result), ec);
            }
        });
    auto handler3 = RecognitionTerminalHandler::create(_stream);
    handler3->onComplete(
        [weakSelf = weak_from_this(), id = _id](wit::Utterances result, std::error_code ec) {
            if (auto self = weakSelf.lock()) {
                LOGD("Terminal handler of <{}> session is complete", id);
                self->onComplete(std::move(result), ec);
            }
        });
    handler2->setNext(std::move(handler3));
    handler1->setNext(std::move(handler2));
    return handler1;
}

void
RecognitionSession::finalize()
{
    if (_onComplete) {
        _onComplete(_id);
    }
}

} // namespace jar
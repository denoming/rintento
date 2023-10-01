#include "intent/RecognitionSession.hpp"

#include "common/Formatters.hpp"
#include "common/IRecognitionFactory.hpp"
#include "intent/AutomationPerformer.hpp"
#include "intent/RecognitionMessageHandler.hpp"
#include "intent/RecognitionSpeechHandler.hpp"
#include "intent/RecognitionTerminalHandler.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>

#include <exception>

namespace jar {

RecognitionSession::Ptr
RecognitionSession::create(std::size_t id,
                           tcp::socket&& socket,
                           std::shared_ptr<IRecognitionFactory> factory,
                           std::shared_ptr<AutomationPerformer> performer)
{
    return Ptr(
        new RecognitionSession{id, std::move(socket), std::move(factory), std::move(performer)});
}

RecognitionSession::RecognitionSession(std::size_t id,
                                       tcp::socket&& socket,
                                       std::shared_ptr<IRecognitionFactory> factory,
                                       std::shared_ptr<AutomationPerformer> performer)
    : _id{id}
    , _stream{std::move(socket)}
    , _factory{std::move(factory)}
    , _performer{std::move(performer)}
{
    BOOST_ASSERT(_id);
    BOOST_ASSERT(_factory);
    BOOST_ASSERT(_performer);
}

std::size_t
RecognitionSession::id() const
{
    return _id;
}

void
RecognitionSession::run()
{
    io::co_spawn(
        _stream.get_executor(),
        [self = shared_from_this()]() {
            LOGD("Run <{}> session", self->id());
            return self->doRun();
        },
        [id = _id](const std::exception_ptr& eptr) {
            try {
                if (eptr) {
                    std::rethrow_exception(eptr);
                }
            } catch (const std::exception& e) {
                LOGE("Unable to run <{}> session: {}", id, e.what());
            }
        });
}

io::awaitable<void>
RecognitionSession::doRun()
{
    LOGD("Read request header: session<{}>", _id);
    std::size_t n = co_await http::async_read_header(_stream, _buffer, _parser, io::use_awaitable);
    LOGD("Reading request header was done: session<{}>, transferred<{}>", _id, n);

    auto handler = getHandler();
    BOOST_ASSERT(handler);
    auto result = co_await handler->handle();
    LOGD("Running <{}> session was complete with <{}> result", _id, result);
    if (result) {
        _performer->perform(result);
    }
}

std::shared_ptr<RecognitionHandler>
RecognitionSession::getHandler()
{
    auto handler1 = RecognitionMessageHandler::create(_stream, _buffer, _parser, _factory);
    auto handler2 = RecognitionSpeechHandler::create(_stream, _buffer, _parser, _factory);
    auto handler3 = RecognitionTerminalHandler::create(_stream);
    handler2->setNext(std::move(handler3));
    handler1->setNext(std::move(handler2));
    return handler1;
}

} // namespace jar
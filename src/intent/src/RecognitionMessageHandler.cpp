#include "intent/RecognitionMessageHandler.hpp"

#include "common/IRecognitionFactory.hpp"
#include "intent/Utils.hpp"

#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/assert.hpp>

using namespace boost::asio::experimental::awaitable_operators;

namespace jar {

RecognitionMessageHandler::Ptr
RecognitionMessageHandler::create(Stream& stream,
                                  Buffer& buffer,
                                  Parser& parser,
                                  std::shared_ptr<IRecognitionFactory> factory)
{
    return Ptr(new RecognitionMessageHandler(stream, buffer, parser, std::move(factory)));
}

RecognitionMessageHandler::RecognitionMessageHandler(Stream& stream,
                                                     Buffer& buffer,
                                                     Parser& parser,
                                                     std::shared_ptr<IRecognitionFactory> factory)
    : RecognitionHandler{stream}
    , _buffer{buffer}
    , _parser{parser}
    , _factory{std::move(factory)}
{
    BOOST_ASSERT(_factory);
}

io::awaitable<RecognitionResult>
RecognitionMessageHandler::handle()
{
    static const std::size_t kChannelCapacity = 64;

    if (not canHandle()) {
        co_return co_await RecognitionHandler::handle();
    }

    auto executor = co_await io::this_coro::executor;
    auto channel = std::make_shared<Channel>(executor, kChannelCapacity);
    auto recognition = _factory->message(executor, channel);
    BOOST_ASSERT(recognition);
    auto result = co_await (sendMessageData(channel) && recognition->run());
    co_await sendResponse(result);
    co_return std::move(result);
}

bool
RecognitionMessageHandler::canHandle() const
{
    return parser::isMessageTarget(_parser.get().target());
}

io::awaitable<void>
RecognitionMessageHandler::sendMessageData(std::shared_ptr<Channel> channel)
{
    const auto request = _parser.release();
    if (auto messageOpt = parser::peekMessage(request.target()); messageOpt) {
        std::ignore = co_await channel->send(io::buffer(*messageOpt));
        co_await channel->send(io::error::eof);
        channel->close();
    } else {
        throw std::runtime_error{"Missing message in request target"};
    }
}

} // namespace jar
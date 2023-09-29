#include "intent/RecognitionSpeechHandler.hpp"

#include "intent/Utils.hpp"
#include "wit/WitRecognitionFactory.hpp"
#include "wit/WitSpeechRecognition.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/assert.hpp>

using namespace boost::asio::experimental::awaitable_operators;

namespace jar {

RecognitionSpeechHandler::Ptr
RecognitionSpeechHandler::create(Stream& stream,
                                 Buffer& buffer,
                                 Parser& parser,
                                 std::shared_ptr<WitRecognitionFactory> factory)
{
    return Ptr(new RecognitionSpeechHandler(stream, buffer, parser, std::move(factory)));
}

RecognitionSpeechHandler::RecognitionSpeechHandler(Stream& stream,
                                                   Buffer& buffer,
                                                   Parser& parser,
                                                   std::shared_ptr<WitRecognitionFactory> factory)
    : RecognitionHandler{stream}
    , _buffer{buffer}
    , _parser{parser}
    , _factory{std::move(factory)}
{
    BOOST_ASSERT(_factory);
}

io::awaitable<wit::Utterances>
RecognitionSpeechHandler::handle()
{
    static const std::size_t kChannelCapacity = 1'000'000 /* 1Mb */;

    if (not canHandle()) {
        co_return co_await RecognitionHandler::handle();
    }

    auto executor = co_await io::this_coro::executor;
    auto channel = std::make_shared<WitSpeechRecognition::Channel>(executor, kChannelCapacity);
    auto recognition = _factory->speech(executor, channel);
    BOOST_ASSERT(recognition);
    auto results = co_await (sendSpeechData(channel) && recognition->run());
    co_await sendResponse(results);
    co_return std::move(results);
}

bool
RecognitionSpeechHandler::canHandle() const
{
    return parser::isSpeechTarget(_parser.get().target());
}

io::awaitable<void>
RecognitionSpeechHandler::sendSpeechData(std::shared_ptr<Channel> channel)
{
    if (auto& request = _parser.get(); request[http::field::expect] != "100-continue") {
        LOGE("100-continue is expected: session");
        throw std::runtime_error{"Missing expect field in request header"};
    }

    http::response<http::empty_body> res{http::status::continue_, net::kHttpVersion11};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    co_await http::async_write(stream(), res, io::use_awaitable);

    std::string chunk;
    auto onHeader = [&](std::uint64_t size, std::string_view extensions, sys::error_code& ec) {
        chunk.reserve(size);
        chunk.clear();
    };
    auto onBody = [&](std::uint64_t remain, std::string_view body, sys::error_code& ec) {
        if (remain == body.size()) {
            ec = http::error::end_of_chunk;
        }
        chunk.append(body.data(), body.size());
        return body.size();
    };
    _parser.on_chunk_header(onHeader);
    _parser.on_chunk_body(onBody);

    sys::error_code ec;
    while (not _parser.is_done()) {
        co_await http::async_read(
            stream(), _buffer, _parser, io::redirect_error(io::use_awaitable, ec));
        if (not ec) {
            continue;
        } else {
            if (ec != http::error::end_of_chunk) {
                LOGE("Error receiving speech data: error<{}>", ec.message());
                break;
            } else {
                ec = {};
            }
        }
        co_await channel->send(io::buffer(chunk));
    }

    channel->close();
}

} // namespace jar
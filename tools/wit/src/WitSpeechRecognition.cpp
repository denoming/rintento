#include "wit/WitSpeechRecognition.hpp"

#include "wit/Utils.hpp"
#include "wit/WitIntentParser.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>

namespace jar {

std::shared_ptr<WitSpeechRecognition>
WitSpeechRecognition::create(io::any_io_executor executor,
                             ssl::context& context,
                             std::string host,
                             std::string port,
                             std::string auth,
                             std::shared_ptr<Channel> channel)
{
    return Ptr(new WitSpeechRecognition(std::move(executor),
                                        context,
                                        std::move(host),
                                        std::move(port),
                                        std::move(auth),
                                        std::move(channel)));
}

WitSpeechRecognition::WitSpeechRecognition(io::any_io_executor executor,
                                           ssl::context& context,
                                           std::string host,
                                           std::string port,
                                           std::string auth,
                                           std::shared_ptr<Channel> channel)
    : WitRecognition{std::move(executor),
                     context,
                     std::move(host),
                     std::move(port),
                     std::move(auth)}
    , _channel{std::move(channel)}
{
    BOOST_ASSERT(_channel);
}

io::awaitable<wit::Utterances>
WitSpeechRecognition::process()
{
    http::request<http::empty_body> req;
    req.version(net::kHttpVersion11);
    req.target(format::speechTargetWithDate());
    req.method(http::verb::post);
    req.set(http::field::host, host());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::authorization, auth());
    req.set(http::field::content_type,
            "audio/raw; encoding=signed-integer; bits=16; rate=16000; endian=little");
    req.set(http::field::transfer_encoding, "chunked");
    req.set(http::field::expect, "100-continue");

    net::resetTimeout(stream());

    LOGD("Write request header");
    http::request_serializer<http::empty_body, http::fields> serializer{req};
    std::size_t n = co_await http::async_write_header(
        stream(), serializer, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Writing request header: bytes<{}>", n);

    net::resetTimeout(stream());

    LOGD("Read response");
    http::response<http::string_body> res;
    beast::flat_buffer buffer;
    n = co_await http::async_read(
        stream(), buffer, res, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Reading response was done: bytes<{}>", n);

    if (res.result() != http::status::continue_) {
        throw std::runtime_error{"Unexpected response status-code result"};
    }

    LOGD("Write audio chunks");
    static const int kMinChunkSize = 512;
    n = 0;
    io::streambuf channelBuffer;
    while (_channel->active() or not _channel->empty()) {
        auto outputSeq = channelBuffer.prepare(kMinChunkSize);
        std::size_t size = co_await _channel->receive(outputSeq);
        channelBuffer.commit(size);
        const auto inputSeq = channelBuffer.data();

        net::resetTimeout(stream());
        n += co_await io::async_write(stream(),
                                      http::make_chunk(inputSeq),
                                      io::bind_cancellation_slot(onCancel(), io::use_awaitable));

        channelBuffer.consume(size);
    }
    LOGD("Writing audio chunk was done: transferred<{}>", n);

    net::resetTimeout(stream());

    LOGD("Write last audio chunk");
    n = co_await io::async_write(stream(),
                                 http::make_chunk_last(),
                                 io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Writing last audio chunk was done: transferred<{}>", n);

    net::resetTimeout(stream());

    LOGD("Read recognition result");
    n = co_await http::async_read(
        stream(), buffer, res, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Reading recognition result was done: transferred<{}>", n);

    if (auto result = jar::WitIntentParser::parseSpeechResult(res.body()); result) {
        co_return std::move(result.value());
    } else {
        throw std::runtime_error{"Unable to parse result"};
    }
}

} // namespace jar
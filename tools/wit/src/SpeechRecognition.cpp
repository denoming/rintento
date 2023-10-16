#include "wit/SpeechRecognition.hpp"

#include "wit/IntentParser.hpp"
#include "wit/Utils.hpp"

#include <jarvisto/Http.hpp>
#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>

namespace jar::wit {

std::shared_ptr<SpeechRecognition>
SpeechRecognition::create(io::any_io_executor executor,
                          ssl::context& context,
                          std::string host,
                          std::string port,
                          std::string auth,
                          std::shared_ptr<Channel> channel)
{
    return Ptr(new SpeechRecognition(std::move(executor),
                                     context,
                                     std::move(host),
                                     std::move(port),
                                     std::move(auth),
                                     std::move(channel)));
}

SpeechRecognition::SpeechRecognition(io::any_io_executor executor,
                                     ssl::context& context,
                                     std::string host,
                                     std::string port,
                                     std::string auth,
                                     std::shared_ptr<Channel> channel)
    : RemoteRecognition{std::move(executor),
                        context,
                        std::move(host),
                        std::move(port),
                        std::move(auth)}
    , _channel{std::move(channel)}
{
    BOOST_ASSERT(_channel);
}

io::awaitable<Utterances>
SpeechRecognition::process()
{
    http::request<http::empty_body> req;
    req.version(kHttpVersion11);
    req.target(speechTargetWithDate());
    req.method(http::verb::post);
    req.set(http::field::host, remoteHost());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::authorization, remoteAuth());
    req.set(http::field::content_type,
            "audio/raw; encoding=signed-integer; bits=16; rate=16000; endian=little");
    req.set(http::field::transfer_encoding, "chunked");
    req.set(http::field::expect, "100-continue");

    resetTimeout(stream());

    LOGD("Write request header");
    http::request_serializer<http::empty_body, http::fields> serializer{req};
    std::size_t n = co_await http::async_write_header(
        stream(), serializer, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Writing request header: bytes<{}>", n);

    resetTimeout(stream());

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
    while (true) {
        onCancel().assign([channel = _channel](auto) {
            LOGD("Close channel upon cancel request");
            channel->close();
        });
        auto outputSeq = channelBuffer.prepare(kMinChunkSize);
        const auto [ec, size] = co_await _channel->recv(outputSeq);
        if (size > 0) {
            channelBuffer.commit(size);
            const auto inputSeq = channelBuffer.data();
            resetTimeout(stream());
            n += co_await io::async_write(
                stream(),
                http::make_chunk(inputSeq),
                io::bind_cancellation_slot(onCancel(), io::use_awaitable));
            channelBuffer.consume(size);
        }
        if (ec) {
            if (ec.value() == io::error::eof) {
                LOGD("End of channel is reached");
                break;
            } else {
                LOGE("Unable to receive message: error<{}>", ec.message());
                throw sys::system_error{io::error::operation_aborted};
            }
        }
    }
    LOGD("Writing audio chunk was done: transferred<{}>", n);

    resetTimeout(stream());

    LOGD("Write last audio chunk");
    n = co_await io::async_write(stream(),
                                 http::make_chunk_last(),
                                 io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Writing last audio chunk was done: transferred<{}>", n);

    resetTimeout(stream());

    LOGD("Read recognition result");
    n = co_await http::async_read(
        stream(), buffer, res, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Reading recognition result was done: transferred<{}>", n);

    if (auto result = IntentParser::parseSpeechResult(res.body()); result) {
        co_return std::move(result.value());
    } else {
        throw std::runtime_error{"Unable to parse result"};
    }
}

} // namespace jar::wit
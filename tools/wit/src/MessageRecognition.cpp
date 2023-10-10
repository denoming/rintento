#include "wit/MessageRecognition.hpp"

#include "wit/IntentParser.hpp"
#include "wit/Utils.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>

namespace jar::wit {

std::shared_ptr<MessageRecognition>
MessageRecognition::create(io::any_io_executor executor,
                           ssl::context& context,
                           std::string host,
                           std::string port,
                           std::string auth,
                           std::shared_ptr<Channel> channel)
{
    return Ptr(new MessageRecognition(std::move(executor),
                                      context,
                                      std::move(host),
                                      std::move(port),
                                      std::move(auth),
                                      std::move(channel)));
}

MessageRecognition::MessageRecognition(io::any_io_executor executor,
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

io::awaitable<wit::Utterances>
MessageRecognition::process()
{
    onCancel().assign([channel = _channel](auto) {
        LOGD("Close channel upon cancel request");
        channel->close();
    });

    std::string message;
    beast::flat_buffer buffer;
    static const size_t kBatchSize = 64;
    while (true) {
        auto outputSeq = buffer.prepare(kBatchSize);
        const auto [ec, size] = co_await _channel->recv(outputSeq);
        if (size > 0) {
            buffer.commit(size);
            const auto inputSeq = buffer.data();
            message.append(static_cast<const char*>(inputSeq.data()), inputSeq.size());
            buffer.consume(size);
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

    http::request<http::empty_body> req;
    req.version(net::kHttpVersion11);
    req.method(http::verb::get);
    req.set(http::field::host, remoteHost());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::authorization, remoteAuth());
    req.set(http::field::content_type, "application/json");
    if (message.empty()) {
        throw std::runtime_error{"Given message is empty"};
    } else {
        req.target(messageTargetWithDate(message));
    }

    net::resetTimeout(stream());

    LOGD("Write request");
    std::size_t n = co_await http::async_write(
        stream(), req, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Writing request was done: transferred<{}>", n);

    net::resetTimeout(stream());

    LOGD("Read recognition result");
    http::response<http::string_body> res;
    n = co_await http::async_read(stream(), buffer, res, io::use_awaitable);
    LOGD("Reading recognition result was done: transferred<{}>", n);

    if (auto result = IntentParser::parseMessageResult(res.body()); result) {
        co_return std::move(result.value());
    } else {
        throw std::runtime_error{"Unable to parse result"};
    }
}

} // namespace jar::wit
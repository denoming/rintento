#include "intent/WitMessageRecognition.hpp"

#include "intent/Utils.hpp"
#include "intent/WitIntentParser.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>

namespace jar {

std::shared_ptr<WitMessageRecognition>
WitMessageRecognition::create(io::any_io_executor executor,
                              ssl::context& context,
                              std::string host,
                              std::string port,
                              std::string auth,
                              std::shared_ptr<Channel> channel)
{
    return Ptr(new WitMessageRecognition(std::move(executor),
                                         context,
                                         std::move(host),
                                         std::move(port),
                                         std::move(auth),
                                         std::move(channel)));
}

WitMessageRecognition::WitMessageRecognition(io::any_io_executor executor,
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
WitMessageRecognition::process()
{
    const auto message = co_await _channel->async_receive(
        io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    if (message.empty()) {
        throw std::runtime_error{"Given message is empty"};
    }

    http::request<http::empty_body> req;
    req.version(net::kHttpVersion11);
    req.method(http::verb::get);
    req.set(http::field::host, host());
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::authorization, auth());
    req.set(http::field::content_type, "application/json");
    req.target(format::messageTargetWithDate(message));

    net::resetTimeout(stream());

    LOGD("Write request");
    std::size_t n = co_await http::async_write(
        stream(), req, io::bind_cancellation_slot(onCancel(), io::use_awaitable));
    LOGD("Writing request was done: transferred<{}>", n);

    net::resetTimeout(stream());

    LOGD("Read recognition result");
    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    n = co_await http::async_read(stream(), buffer, res, io::use_awaitable);
    LOGD("Reading recognition result was done: transferred<{}>", n);

    if (auto result = jar::WitIntentParser::parseMessageResult(res.body()); result) {
        co_return std::move(result.value());
    } else {
        throw std::runtime_error{"Unable to parse result"};
    }
}

} // namespace jar
#include "intent/RecognitionHandler.hpp"

#include <jarvisto/core/Logger.hpp>
#include <jarvisto/network/Http.hpp>

#include <boost/assert.hpp>
#include <boost/json.hpp>

namespace json = boost::json;

namespace jar {

namespace {

std::string
getPayload(const RecognitionResult& /*result*/)
{
    json::value value;
    auto& object = value.emplace_object();
    object.emplace("status", true);
    return json::serialize(value);
}

std::string
getPayload(std::error_code error)
{
    json::value value;
    auto& object = value.emplace_object();
    object.emplace("status", false);
    object.emplace("error", error.message());
    return json::serialize(value);
}

http::response<http::string_body>
getResponse(std::string payload)
{
    http::response<http::string_body> response{http::status::ok, kHttpVersion11};
    response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(http::field::content_type, "application/json");
    response.keep_alive(false);
    response.body() = std::move(payload);
    response.prepare_payload();
    return response;
}

} // namespace

RecognitionHandler::RecognitionHandler(Stream& stream)
    : _stream{stream}
{
}

void
RecognitionHandler::setNext(std::shared_ptr<RecognitionHandler> handler)
{
    BOOST_ASSERT(!_next);
    _next = std::move(handler);
}

io::awaitable<RecognitionResult>
RecognitionHandler::handle()
{
    if (_next) {
        BOOST_ASSERT(_next.get() != this);
        co_return co_await _next->handle();
    } else {
        throw std::logic_error{"No handler"};
    }
}

io::awaitable<void>
RecognitionHandler::sendResponse(const RecognitionResult& result)
{
    auto response = getResponse(getPayload(result));
    std::ignore = co_await http::async_write(stream(), response, io::as_tuple(io::use_awaitable));
}

io::awaitable<void>
RecognitionHandler::sendResponse(std::error_code ec)
{
    auto response = getResponse(getPayload(ec));
    std::ignore = co_await http::async_write(stream(), response, io::as_tuple(io::use_awaitable));
}

RecognitionHandler::Stream&
RecognitionHandler::stream()
{
    return _stream;
}

io::any_io_executor
RecognitionHandler::executor()
{
    return _stream.get_executor();
}

} // namespace jar
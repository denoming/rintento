#include "intent/RecognitionHandler.hpp"

#include <jarvisto/Logger.hpp>

#include <boost/assert.hpp>
#include <boost/json.hpp>

namespace json = boost::json;

namespace jar {

namespace {

std::string
getPayload(const wit::Utterances& /*result*/)
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
    http::response<http::string_body> response{http::status::ok, net::kHttpVersion11};
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
RecognitionHandler::onComplete(std::move_only_function<OnComplete> callback)
{
    _onComplete = std::move(callback);
}

void
RecognitionHandler::setNext(std::shared_ptr<RecognitionHandler> handler)
{
    BOOST_ASSERT(!_next);
    _next = std::move(handler);
}

void
RecognitionHandler::handle()
{
    if (_next) {
        _next->handle();
    }
}

void
RecognitionHandler::submit(wit::Utterances result)
{
    BOOST_ASSERT(_onComplete);
    _onComplete(std::move(result), {});
}

void
RecognitionHandler::submit(std::error_code ec)
{
    BOOST_ASSERT(_onComplete);
    _onComplete({}, ec);
}

void
RecognitionHandler::sendResponse(const wit::Utterances& result)
{
    sys::error_code ec;
    http::write(stream(), getResponse(getPayload(result)), ec);
}

void
RecognitionHandler::sendResponse(std::error_code ec)
{
    sys::error_code ec1;
    http::write(stream(), getResponse(getPayload(ec)), ec1);
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
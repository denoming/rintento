#include "intent/RecognitionHandler.hpp"

#include "common/Logger.hpp"
#include "intent/RecognitionConnection.hpp"

#include <boost/json.hpp>

namespace json = boost::json;

namespace jar {

namespace {

std::string
getPayload(const Utterances& result)
{
    json::value value;
    auto& object = value.emplace_object();
    object.emplace("status", true);
    return json::serialize(value);
}

std::string
getPayload(sys::error_code error)
{
    json::value value;
    auto& object = value.emplace_object();
    object.emplace("status", false);
    object.emplace("error", error.to_string());
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

RecognitionHandler::RecognitionHandler(std::shared_ptr<RecognitionConnection> connection)
    : _connection{std::move(connection)}
{
    assert(_connection);
}

void
RecognitionHandler::setNext(std::shared_ptr<RecognitionHandler> handler)
{
    assert(!_next);
    _next = std::move(handler);
}

void
RecognitionHandler::handle(Buffer& buffer, Parser& parser)
{
    if (_next) {
        _next->handle(buffer, parser);
    }
}

RecognitionConnection&
RecognitionHandler::connection()
{
    return *_connection;
}

const RecognitionConnection&
RecognitionHandler::connection() const
{
    return *_connection;
}

void
RecognitionHandler::submit(Utterances result)
{
    assert(_doneCallback);
    _doneCallback(std::move(result), {});
}

void
RecognitionHandler::submit(sys::error_code error)
{
    assert(_doneCallback);
    _doneCallback({}, error);
}

void
RecognitionHandler::sendResponse(const Utterances& result)
{
    _connection->write(getResponse(getPayload(result)));
}

void
RecognitionHandler::sendResponse(sys::error_code error)
{
    _connection->write(getResponse(getPayload(error)));
}

} // namespace jar
#include "intent/IntentRecognizeHandler.hpp"

#include "common/Logger.hpp"

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

IntentRecognizeHandler::IntentRecognizeHandler(IntentRecognizeConnection::Ptr connection,
                                               Callback callback)
    : _connection{std::move(connection)}
    , _callback{std::move(callback)}
{
    assert(_connection);
    assert(_callback);
}

void
IntentRecognizeHandler::setNext(Ptr handler)
{
    assert(!_next);
    _next = std::move(handler);
}

void
IntentRecognizeHandler::handle(Buffer& buffer, Parser& parser)
{
    if (_next) {
        _next->handle(buffer, parser);
    }
}

IntentRecognizeConnection&
IntentRecognizeHandler::connection()
{
    return *_connection;
}

const IntentRecognizeConnection&
IntentRecognizeHandler::connection() const
{
    return *_connection;
}

void
IntentRecognizeHandler::submit(Utterances result)
{
    _callback(std::move(result), {});
}

void
IntentRecognizeHandler::submit(sys::error_code error)
{
    _callback({}, error);
}

void
IntentRecognizeHandler::sendResponse(const Utterances& result)
{
    _connection->write(getResponse(getPayload(result)));
}

void
IntentRecognizeHandler::sendResponse(sys::error_code error)
{
    _connection->write(getResponse(getPayload(error)));
}

} // namespace jar
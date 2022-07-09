#include "intent/IntentRecognizeMessage.hpp"

#include "intent/WitMessageRecognition.hpp"
#include "intent/WitPendingRecognition.hpp"
#include "common/Logger.hpp"
#include "intent/Http.hpp"

#include <boost/json/value.hpp>
#include <boost/json/serialize.hpp>

namespace json = boost::json;

namespace jar {

std::string
getPayload(const Utterances& utterances)
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
    object.emplace("status", true);
    object.emplace("error", error.to_string());
    return json::serialize(value);
}

http::response<http::string_body>
getResponse(const std::string& payload)
{
    http::response<http::string_body> response{http::status::ok, kHttpVersion11};
    response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(http::field::content_type, "application/json");
    response.keep_alive(false);
    response.body() = payload;
    response.prepare_payload();
    return response;
}

IntentRecognizeMessage::IntentRecognizeMessage(WitMessageRecognition::Ptr recognition,
                                               IntentRecognizeConnection::Ptr connection,
                                               std::string_view message)
    : _recognition{recognition}
    , _connection{connection}
    , _message{message}
{
    assert(recognition);
    assert(connection);
}

void
IntentRecognizeMessage::execute(Callback callback)
{
    assert(callback);
    _callback = std::move(callback);

    _pending = WitPendingRecognition::create(
        _recognition,
        [this](auto result, auto error) { onComplete(std::move(result), error); },
        _connection->executor());

    _recognition->run(_message);
}

void
IntentRecognizeMessage::onComplete(Utterances result, sys::error_code error)
{
    if (error) {
        LOGE("Failed to recognize message: <{}>", error.what());
    }

    auto response = getResponse(error ? getPayload(error) : getPayload(result));
    _connection->write(std::move(response),
                       [callback = std::move(_callback), result = std::move(result), error](auto) {
                           callback(std::move(result), error);
                       });
}

} // namespace jar
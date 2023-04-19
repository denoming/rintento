#include "intent/WitIntentParser.hpp"

#include <boost/assert.hpp>
#include <boost/json.hpp>

namespace json = boost::json;

namespace jar {

namespace {

std::optional<IntentSpec>
toIntent(const json::object& jsonObject)
{
    const auto nameValue = jsonObject.if_contains("name");
    const auto confValue = jsonObject.if_contains("confidence");
    if ((nameValue && nameValue->is_string()) && (confValue && confValue->is_double())) {
        std::string name{nameValue->as_string().begin(), nameValue->as_string().end()};
        return IntentSpec{std::move(name), static_cast<float>(confValue->as_double())};
    }
    return std::nullopt;
}

IntentSpecs
toIntents(const json::array& jsonIntents)
{
    IntentSpecs intents;
    for (auto jsonIntent : jsonIntents) {
        if (const auto intentObject = jsonIntent.if_object(); intentObject) {
            if (auto intentOpt = toIntent(*intentObject); intentOpt) {
                intents.push_back(std::move(*intentOpt));
            }
        }
    }
    return intents;
}

std::optional<UtteranceSpec>
toUtterance(const json::object& object, const bool finalByDefault = false)
{
    IntentSpecs intents;
    if (auto intentsValue = object.if_contains("intents"); intentsValue) {
        if (auto intentsArray = intentsValue->if_array(); intentsArray) {
            intents = toIntents(*intentsArray);
        }
    }

    bool isFinal{finalByDefault};
    if (auto finalValue = object.if_contains("is_final"); finalValue) {
        if (auto finalPtr = finalValue->if_bool(); finalPtr) {
            isFinal = *finalPtr;
        }
    }

    std::string text;
    if (auto textValue = object.if_contains("text"); textValue) {
        if (auto textPtr = textValue->if_string(); textPtr) {
            text.assign(textPtr->cbegin(), textPtr->cend());
        }
    }

    return UtteranceSpec{std::move(text), std::move(intents), isFinal};
}

} // namespace

std::expected<UtteranceSpecs, std::error_code>
WitIntentParser::parseMessageResult(std::string_view input)
{
    if (input.empty()) {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    std::error_code error;
    json::value value = json::parse(input, error);
    if (error) {
        return std::unexpected(error);
    }

    UtteranceSpecs output;
    if (auto object = value.if_object(); object) {
        if (auto utterance = toUtterance(*object, true); utterance) {
            output.push_back(std::move(*utterance));
        }
    }
    return output;
}

std::expected<UtteranceSpecs, std::error_code>
WitIntentParser::parseSpeechResult(std::string_view input)
{
    if (input.empty()) {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    UtteranceSpecs output;
    json::stream_parser parser;
    std::size_t nextRoot{0};
    do {
        std::error_code error;
        if (nextRoot = parser.write_some(input, error); error) {
            return std::unexpected(error);
        }
        if (nextRoot > 0) {
            input = input.substr(nextRoot);
            auto value = parser.release();
            if (auto object = value.if_object(); object) {
                if (auto utterance = toUtterance(*object); utterance) {
                    output.push_back(std::move(*utterance));
                }
            }
        }
    }
    while (nextRoot > 0);
    return output;
}

} // namespace jar

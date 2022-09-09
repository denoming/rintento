#include "intent/WitIntentParser.hpp"

#include "common/Logger.hpp"
#include "intent/Http.hpp"

#include <optional>

namespace jar {

namespace {

std::optional<Intent>
toIntent(const json::object& jsonObject)
{
    const auto nameValue = jsonObject.if_contains("name");
    const auto confValue = jsonObject.if_contains("confidence");
    if ((nameValue && nameValue->is_string()) && (confValue && confValue->is_double())) {
        std::string name{nameValue->as_string().begin(), nameValue->as_string().end()};
        return Intent{std::move(name), static_cast<float>(confValue->as_double())};
    }
    return std::nullopt;
}

Intents
toIntents(const json::array& jsonIntents)
{
    Intents intents;
    for (auto jsonIntent : jsonIntents) {
        if (const auto intentObject = jsonIntent.if_object(); intentObject) {
            if (auto intentOpt = toIntent(*intentObject); intentOpt) {
                intents.push_back(std::move(*intentOpt));
            }
        }
    }
    return intents;
}

std::optional<Utterance>
toUtterance(const json::object& object)
{
    auto intentsObject = object.if_contains("intents");
    if (!intentsObject) {
        return std::nullopt;
    }
    Intents intents;
    if (auto intentsArray = intentsObject->if_array(); intentsArray) {
        intents = toIntents(*intentsArray);
    }
    if (intents.empty()) {
        return std::nullopt;
    }

    auto textObject = object.if_contains("text");
    if (!textObject) {
        return std::nullopt;
    }
    std::string text{textObject->as_string().begin(), textObject->as_string().end()};
    return Utterance{std::move(text), std::move(intents)};
}

} // namespace

std::optional<Utterances>
WitIntentParser::parse(std::string_view input)
{
    _parser.reset();

    if (input.empty()) {
        LOGE("Input value is empty");
        return std::nullopt;
    }

    Utterances utterances;
    std::size_t nextRoot{0};
    do {
        sys::error_code error;
        nextRoot = _parser.write_some(input, error);
        if (error) {
            LOGE("Failed to parse: {}", error.message());
            return std::nullopt;
        }
        if (!nextRoot) {
            break;
        }
        input = input.substr(nextRoot);
        auto value = _parser.release();
        if (auto utteranceObject = value.if_object(); utteranceObject) {
            if (auto utteranceOpt = toUtterance(*utteranceObject); utteranceOpt) {
                utterances.push_back(std::move(*utteranceOpt));
            }
        }
    }
    while (nextRoot > 0);
    return utterances;
}

} // namespace jar

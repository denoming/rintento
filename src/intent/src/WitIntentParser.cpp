#include "intent/WitIntentParser.hpp"

#include "jarvis/Logger.hpp"

#include <boost/assert.hpp>
#include <boost/json/stream_parser.hpp>

#include <optional>

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
toUtterance(const json::object& object)
{
    IntentSpecs intents;
    if (auto intentsValue = object.if_contains("intents"); intentsValue) {
        if (auto intentsArray = intentsValue->if_array(); intentsArray) {
            intents = toIntents(*intentsArray);
        }
    }

    bool isFinal{false};
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

class WitIntentParser::Impl {
public:
    std::optional<UtteranceSpecs>
    parse(std::string_view input)
    {
        _parser.reset();

        if (input.empty()) {
            LOGE("Input value is empty");
            return std::nullopt;
        }

        UtteranceSpecs utterances;
        std::size_t nextRoot{0};
        do {
            std::error_code error;
            if (nextRoot = _parser.write_some(input, error); error) {
                LOGE("Failed to parse: {}", error.message());
                return std::nullopt;
            }
            if (nextRoot > 0) {
                input = input.substr(nextRoot);
                auto value = _parser.release();
                if (auto utteranceObject = value.if_object(); utteranceObject) {
                    if (auto utteranceOpt = toUtterance(*utteranceObject); utteranceOpt) {
                        utterances.push_back(std::move(*utteranceOpt));
                    }
                }
            }
        }
        while (nextRoot > 0);
        return utterances;
    }

private:
    json::stream_parser _parser;
};

WitIntentParser::WitIntentParser()
    : _impl{std::make_unique<Impl>()}
{
}

WitIntentParser::~WitIntentParser() = default;

std::optional<UtteranceSpecs>
WitIntentParser::parse(std::string_view input)
{
    BOOST_ASSERT(_impl);
    return _impl->parse(input);
}

} // namespace jar

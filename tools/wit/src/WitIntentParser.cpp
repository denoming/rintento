#include "wit/WitIntentParser.hpp"

#include <jarvisto/DateTime.hpp>

#include <boost/json.hpp>

namespace json = boost::json;

namespace jar {

static Timestamp
tag_invoke(json::value_to_tag<Timestamp>, const json::value& v)
{
    if (auto ts = parseZonedDateTime(v.as_string()); ts.has_value()) {
        return ts.value();
    } else {
        throw std::system_error{ts.error()};
    }
}

namespace wit {

static Confidence
tag_invoke(json::value_to_tag<Confidence>, const json::value& v)
{
    if (auto c = v.if_double(); c) {
        return {.value = static_cast<float>(*c)};
    }
    if (auto c = v.if_int64(); c) {
        return {.value = static_cast<float>(*c)};
    }
    throw std::out_of_range{"Invalid confidence value"};
}

static DateTimeEntity::Grains
tag_invoke(json::value_to_tag<DateTimeEntity::Grains>, const json::value& v)
{
    const auto& str = v.as_string();
    if (str == "hour") {
        return DateTimeEntity::Grains::hour;
    }
    if (str == "day") {
        return DateTimeEntity::Grains::day;
    }
    return DateTimeEntity::Grains::unknown;
}

static DateTimeEntity::Value
tag_invoke(json::value_to_tag<DateTimeEntity::Value>, const json::value& v)
{
    DateTimeEntity::Value value;
    value.grain = json::value_to<DateTimeEntity::Grains>(v.as_object().at("grain"));
    value.timestamp = json::value_to<Timestamp>(v.as_object().at("value"));
    return value;
}

static DateTimeEntity
tag_invoke(json::value_to_tag<DateTimeEntity>, const json::value& v)
{
    DateTimeEntity entity;
    const auto& object = v.as_object();
    if (const auto& value = object.if_contains("value"); value) {
        entity.valueFrom = entity.valueTo = json::value_to<DateTimeEntity::Value>(v);
    } else {
        if (const auto& from = object.if_contains("from"); from) {
            entity.valueFrom = json::value_to<DateTimeEntity::Value>(*from);
        }
        if (const auto& to = object.if_contains("to"); to) {
            entity.valueTo = json::value_to<DateTimeEntity::Value>(*to);
        }
    }
    entity.confidence = json::value_to<Confidence>(object.at("confidence"));
    return entity;
}

static std::optional<EntityAlts>
toEntity(const json::value& input)
{
    try {
        const auto& name = input.at("name").as_string();
        const auto& role = input.at("role").as_string();
        if (name == DateTimeEntity::kName && role == DateTimeEntity::kRole) {
            return json::value_to<DateTimeEntity>(input);
        }
        return std::nullopt;
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

static EntityList
toEntities(const json::value& input)
{
    EntityList entityList;
    for (auto&& entry : input.as_array()) {
        if (auto entity = toEntity(entry); entity) {
            entityList.push_back(std::move(*entity));
        }
    }
    return entityList;
}

static std::optional<Intent>
toIntent(const json::object& input)
{
    try {
        const auto& name = input.at("name").as_string();
        const auto& conf = input.at("confidence").as_double();
        return Intent{std::string(name.cbegin(), name.cend()), static_cast<float>(conf)};
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

static Intents
toIntents(const json::array& input)
{
    Intents intents;
    for (auto entry : input) {
        if (const auto object = entry.if_object(); object) {
            if (auto intent = toIntent(*object); intent) {
                intents.push_back(std::move(*intent));
            }
        }
    }
    return intents;
}

static std::optional<Utterance>
toUtterance(const json::object& root, const bool finalByDefault = false)
{
    try {
        Entities entities;
        if (auto value = root.if_contains("entities"); value) {
            for (auto&& entry : value->as_object()) {
                entities[entry.key()] = toEntities(entry.value());
            }
        }

        Intents intents;
        if (auto value = root.if_contains("intents"); value) {
            intents = toIntents(value->as_array());
        }

        bool isFinal{finalByDefault};
        if (auto value = root.if_contains("is_final"); value) {
            isFinal = value->as_bool();
        }

        std::string text;
        if (auto value = root.if_contains("text"); value) {
            text = value->as_string();
        }

        return Utterance{std::move(text), std::move(entities), std::move(intents), isFinal};
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

} // namespace wit

std::expected<wit::Utterances, std::error_code>
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

    wit::Utterances output;
    if (auto object = value.if_object(); object) {
        if (auto utterance = wit::toUtterance(*object, true); utterance) {
            output.push_back(std::move(*utterance));
        }
    }
    return output;
}

std::expected<wit::Utterances, std::error_code>
WitIntentParser::parseSpeechResult(std::string_view input)
{
    if (input.empty()) {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    wit::Utterances output;
    json::stream_parser parser;
    std::size_t nextRoot;
    do {
        std::error_code error;
        if (nextRoot = parser.write_some(input, error); error) {
            return std::unexpected(error);
        }
        if (nextRoot > 0) {
            input = input.substr(nextRoot);
            auto value = parser.release();
            if (auto object = value.if_object(); object) {
                if (auto utterance = wit::toUtterance(*object); utterance) {
                    output.push_back(std::move(*utterance));
                }
            }
        }
    }
    while (nextRoot > 0);
    return output;
}

} // namespace jar

#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace jar {

struct Confidence {
    float value{};
};

struct Timestamp {
    Timestamp() = default;

    Timestamp(const int64_t v);

    Timestamp(std::chrono::sys_seconds v);

    auto
    operator<=>(const Timestamp& other) const
        = default;

    operator std::chrono::sys_seconds() const;

    static Timestamp
    zero();

    static Timestamp
    now();

    int64_t value{};
};

struct Entity {
    Confidence confidence{};
};

struct DateTimeEntity {
    inline static const std::string_view kName{"wit$datetime"};
    inline static const std::string_view kRole{"datetime"};

    enum class Grains { unknown, hour, day };

    struct Value {
        Grains grain{Grains::unknown};
        Timestamp timestamp;
    };

    static std::string
    key()
    {
        return std::string{kName} + ':' + std::string{kRole};
    }

    Confidence confidence;
    std::optional<Value> from;
    std::optional<Value> to;
    std::optional<Value> exact;
};

using EntityAlts = std::variant<std::monostate, DateTimeEntity>;

using EntityList = std::vector<EntityAlts>;

using Entities = std::unordered_map<std::string, EntityList>;

/**
 * The intent class representation
 */
struct Intent {
    std::string name;
    float confidence{0.0f};
};

/** The bunch of intents */
using Intents = std::vector<Intent>;

/**
 * The utterance class representation
 */
struct Utterance {
    std::string text;
    Entities entities;
    Intents intents;
    bool final{false};
};

/** The bunch of utterances */
using Utterances = std::vector<Utterance>;

bool
operator==(const Intent& lhs, const Intent& rhs);

} // namespace jar

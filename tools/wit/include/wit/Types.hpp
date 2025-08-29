// Copyright 2025 Denys Asauliak
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <jarvisto/core/Timestamp.hpp>

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace jar::wit {

/**
 * The confidence type.
 */
struct Confidence {
    float value{};
};

/**
 * The base for all entities.
 */
struct Entity {
    Confidence confidence{};
};

/**
 * Date and time entity type
 * (used to store recognized date and time in a speech)
 */
struct DateTimeEntity : Entity {
    inline static const std::string_view kName{"wit$datetime"};
    inline static const std::string_view kRole{"datetime"};

    enum class Grains { unknown, hour, day };

    struct Value {
        Grains grain{Grains::unknown};
        Timestamp timestamp;
    };

    [[nodiscard]] static std::string
    key()
    {
        return std::string{kName} + ':' + std::string{kRole};
    }

    [[nodiscard]] bool
    hasValue() const
    {
        return (valueFrom && valueTo);
    }

    [[nodiscard]] const Timestamp&
    timestampFrom() const
    {
        return valueFrom->timestamp;
    }

    [[nodiscard]] const Timestamp&
    timestampTo() const
    {
        return valueTo->timestamp;
    }

    std::optional<Value> valueFrom;
    std::optional<Value> valueTo;
};

/**
 * Ordinary entity type
 * (used to store recognized ordinary number in a speech)
 */
struct OrdinaryEntity : Entity {
    inline static const std::string_view kName{"wit$ordinal"};
    inline static const std::string_view kRole{"ordinal"};

    static std::string
    key()
    {
        return std::string{kName} + ':' + std::string{kRole};
    }

    [[nodiscard]] bool
    hasValue() const
    {
        return bool(value);
    }

    [[nodiscard]] int32_t
    number() const
    {
        return *value;
    }

    std::optional<int32_t> value;
};

/* The type represents all possible entity types */
using EntityAlts = std::variant<std::monostate, DateTimeEntity, OrdinaryEntity>;

/* The list of all recognized entities */
using EntityList = std::vector<EntityAlts>;

/* The list of all recognized entities indexed by key */
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

} // namespace jar::wit

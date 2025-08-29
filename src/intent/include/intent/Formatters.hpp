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

#include "intent/WitTypes.hpp"

#include <fmt/format.h>

template<>
struct fmt::formatter<jar::wit::Intent> {
    constexpr auto
    parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto
    format(const jar::wit::Intent& i, FormatContext& c) const
    {
        constexpr const std::string_view kFormat{"name<{}>, confidence<{:.3f}>"};
        return fmt::format_to(c.out(), kFormat, i.name, i.confidence);
    }
};

template<>
struct fmt::formatter<jar::wit::Intents> : fmt::formatter<jar::wit::Intent> {
    template<typename FormatContext>
    auto
    format(const jar::wit::Intents& is, FormatContext& c) const
    {
        return fmt::format_to(c.out(), "({})", fmt::join(is, "), ("));
    }
};

template<>
struct fmt::formatter<jar::wit::Utterance> {
    constexpr auto
    parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto
    format(const jar::wit::Utterance& u, FormatContext& c) const
    {
        constexpr const std::string_view kFormat{"text<{}>, intents<({})>, final<{}>"};
        return fmt::format_to(c.out(), kFormat, u.text, fmt::join(u.intents, "), ("), u.final);
    }
};

template<>
struct fmt::formatter<jar::wit::Utterances> : fmt::formatter<jar::wit::Utterance> {
    template<typename FormatContext>
    auto
    format(const jar::wit::Utterances& us, FormatContext& c) const
    {
        return fmt::format_to(c.out(), "({})", fmt::join(us, "), ("));
    }
};

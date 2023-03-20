#pragma once

#include "intent/Types.hpp"

#include <fmt/format.h>

template<>
struct fmt::formatter<jar::IntentSpec> : fmt::formatter<std::string_view> {
    template<typename FormatContext>
    auto
    format(const jar::IntentSpec& i, FormatContext& c) const
    {
        constexpr const std::string_view kFormat{"name<{}>, confidence<{:.3f}>"};
        return fmt::format_to(c.out(), kFormat, i.name, i.confidence);
    }
};

template<>
struct fmt::formatter<jar::IntentSpecs> : fmt::formatter<jar::IntentSpec> {
    template<typename FormatContext>
    auto
    format(const jar::IntentSpecs& is, FormatContext& c) const
    {
        return fmt::format_to(c.out(), "({})", fmt::join(is, "), ("));
    }
};

template<>
struct fmt::formatter<jar::UtteranceSpec> : fmt::formatter<jar::IntentSpec> {
    template<typename FormatContext>
    auto
    format(const jar::UtteranceSpec& u, FormatContext& c) const
    {
        constexpr const std::string_view kFormat{"text<{}>, intents<({})>, final<{}>"};
        return fmt::format_to(c.out(), kFormat, u.text, fmt::join(u.intents, "), ("), u.final);
    }
};

template<>
struct fmt::formatter<jar::UtteranceSpecs> : fmt::formatter<jar::UtteranceSpec> {
    template<typename FormatContext>
    auto
    format(const jar::UtteranceSpecs& us, FormatContext& c) const
    {
        return fmt::format_to(c.out(), "({})", fmt::join(us, "), ("));
    }
};
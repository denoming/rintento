#pragma once

#include "intent/Types.hpp"
#include "intent/WitTypes.hpp"

#include <fmt/format.h>

template<>
struct fmt::formatter<jar::Intent> {
    constexpr auto
    parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto
    format(const jar::Intent& i, FormatContext& c) const
    {
        constexpr const std::string_view kFormat{"name<{}>, confidence<{:.3f}>"};
        return fmt::format_to(c.out(), kFormat, i.name, i.confidence);
    }
};

template<>
struct fmt::formatter<jar::Intents> : fmt::formatter<jar::Intent> {
    template<typename FormatContext>
    auto
    format(const jar::Intents& is, FormatContext& c) const
    {
        return fmt::format_to(c.out(), "({})", fmt::join(is, "), ("));
    }
};

template<>
struct fmt::formatter<jar::Utterance> {
    constexpr auto
    parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto
    format(const jar::Utterance& u, FormatContext& c) const
    {
        constexpr const std::string_view kFormat{"text<{}>, intents<({})>, final<{}>"};
        return fmt::format_to(c.out(), kFormat, u.text, fmt::join(u.intents, "), ("), u.final);
    }
};

template<>
struct fmt::formatter<jar::Utterances> : fmt::formatter<jar::Utterance> {
    template<typename FormatContext>
    auto
    format(const jar::Utterances& us, FormatContext& c) const
    {
        return fmt::format_to(c.out(), "({})", fmt::join(us, "), ("));
    }
};

template<>
struct fmt::formatter<jar::GeoLocation> {
    constexpr auto
    parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto
    format(const jar::GeoLocation& loc, FormatContext& c) const
    {
        constexpr const std::string_view kFormat{"(lat: <{:.3f}>, lon: <{:.3f}>, alt: <{:.3f}>)"};
        return fmt::format_to(c.out(), kFormat, loc.lat, loc.lon, loc.alt);
    }
};
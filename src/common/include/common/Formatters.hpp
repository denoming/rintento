#pragma once

#include "common/Types.hpp"

#include <fmt/format.h>

template<>
struct fmt::formatter<jar::RecognitionResult> {
    constexpr auto
    parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto
    format(const jar::RecognitionResult& result, FormatContext& ctx) const
    {
        constexpr const std::string_view kFormat{"intent<{}>"};
        return fmt::format_to(ctx.out(), kFormat, result ? result.intent : "?");
    }
};
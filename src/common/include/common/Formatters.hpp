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
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

#include "intent/Utils.hpp"

#include <jarvisto/core/Logger.hpp>

#include <boost/url/encode.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/rfc/pchars.hpp>

#include <spdlog/fmt/chrono.h>
#include <spdlog/fmt/fmt.h>

namespace urls = boost::urls;

using namespace std::chrono;

namespace jar {

namespace format {

std::string
messageTarget(std::string_view message)
{
    static constexpr std::string_view kFormat{"/message?q={}"};
    urls::encoding_opts opts;
    opts.space_as_plus = true;
    return fmt::format(fmt::runtime(kFormat), encode(message, urls::pchars, opts));
}

std::string
messageTargetWithDate(std::string_view message)
{
    static constexpr std::string_view kFormat{"/message?v={:%Y%m%d}&q={}"};
    urls::encoding_opts opts;
    opts.space_as_plus = true;
    return fmt::format(kFormat, system_clock::now(), encode(message, urls::pchars, opts));
}

std::string
speechTarget()
{
    static constexpr std::string_view kFormat{"/speech"};
    return std::string{kFormat};
}

std::string
speechTargetWithDate()
{
    static constexpr std::string_view kFormat{"/speech?v={:%Y%m%d}"};
    return fmt::format(kFormat, system_clock::now());
}

} // namespace format

namespace parser {

std::optional<std::string>
peekMessage(std::string_view target)
{
    if (auto view = urls::parse_origin_form(target); view->has_query()) {
        urls::encoding_opts opts;
        opts.space_as_plus = true;
        auto params = view->params(opts);
        if (auto queryIt = params.find("q"); queryIt != params.end()) {
            const auto& query = *queryIt;
            if (query.has_value) {
                return query.value;
            }
        }
    }
    return std::nullopt;
}

bool
isMessageTarget(std::string_view input)
{
    static constexpr std::string_view kPrefix{"/message"};
    return input.starts_with(kPrefix);
}

bool
isSpeechTarget(std::string_view input)
{
    static constexpr std::string_view kPrefix{"/speech"};
    return input.starts_with(kPrefix);
}

} // namespace parser

} // namespace jar

#include "intent/Utils.hpp"

#include <boost/url/urls.hpp>

#include <fmt/format.h>
#include <fmt/chrono.h>

namespace urls = boost::urls;

using namespace std::chrono;

namespace jar {

namespace pct {

std::string
encode(std::string_view input)
{
    const urls::pct_encode_opts opts = {
        .space_to_plus = true,
    };
    return urls::pct_encode_to_value(input, opts, urls::pchars);
}

std::string
decode(std::string_view input)
{
    const urls::pct_decode_opts opts = {
        .plus_to_space = true,
    };
    return urls::pct_decode_to_value(input, opts, urls::pchars);
}

} // namespace pct

namespace format {

std::string
messageTarget(std::string_view message)
{
    static constexpr std::string_view kFormat{"/message?q={}"};
    return fmt::format(fmt::runtime(kFormat), pct::encode(message));
}

std::string
messageTargetWithDate(std::string_view message)
{
    static constexpr std::string_view kFormat{"/message?v={:%Y%m%d}&q={}"};
    return fmt::format(fmt::runtime(kFormat), system_clock::now(), pct::encode(message));
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

namespace parse {

std::optional<std::string>
messageTarget(std::string_view target)
{
    static constexpr std::string_view MessageQuery{"/message"};

    if (!target.starts_with(MessageQuery)) {
        return std::nullopt;
    }

    auto params = urls::parse_query_params(target.substr(MessageQuery.size() + 1));
    if (!params || params->empty()) {
        return std::nullopt;
    }

    const auto decodedParams = params->decoded();
    if (auto queryItemIt = decodedParams.find("q"); queryItemIt != decodedParams.end()) {
        const auto& queryItem = *queryItemIt;
        if (queryItem.has_value) {
            return queryItem.value;
        }
    }

    return std::nullopt;
}

} // namespace parse

} // namespace jar

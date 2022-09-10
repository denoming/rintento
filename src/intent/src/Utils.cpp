#include "intent/Utils.hpp"

#include "boost/url/rfc/pchars.hpp"
#include <boost/url/pct_encoding.hpp>
#include <boost/url/urls.hpp>

#include <spdlog/fmt/chrono.h>
#include <spdlog/fmt/fmt.h>

#include <vector>

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
    return urls::pct_encode_to_string(input, urls::pchars, opts);
}

std::string
decode(std::string_view input)
{
    const urls::pct_decode_opts opts = {
        .plus_to_space = true,
    };
    std::vector<char> buffer(input.size() * 2, '\0');
    const auto r = urls::pct_decode(buffer.data(), buffer.data() + buffer.size(), input, opts);
    return r.has_value() ? std::string(buffer.data(), r.value()) : "";
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

namespace parser {

std::optional<std::string>
peekMessage(std::string_view target)
{
    if (const auto pos = target.find_first_of('?'); pos != std::string_view::npos) {
        const auto params = urls::parse_query_params(target.substr(pos + 1));
        const auto decodedParams = params->decoded();
        if (auto queryItemIt = decodedParams.find("q"); queryItemIt != decodedParams.end()) {
            if (const auto& queryItem = *queryItemIt; queryItem.has_value) {
                std::string output;
                queryItem.value.assign_to(output);
                return output;
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

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

} // namespace jar

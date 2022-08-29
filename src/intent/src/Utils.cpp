#include "intent/Utils.hpp"

#include <boost/url/urls.hpp>
#include <boost/url/pct_encoding.hpp>
#include "boost/url/rfc/pchars.hpp"

#include <fmt/format.h>
#include <fmt/chrono.h>

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

} // namespace jar

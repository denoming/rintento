#include "wit/Utils.hpp"

#include <boost/url/encode.hpp>
#include <boost/url/rfc/pchars.hpp>

#include <spdlog/fmt/chrono.h>
#include <spdlog/fmt/fmt.h>

namespace urls = boost::urls;

using namespace std::chrono;

namespace jar::wit {

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

} // namespace jar::wit
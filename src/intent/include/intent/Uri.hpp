#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <concepts>

namespace jar {

namespace detail {

class Plus {
public:
    static std::string
    encodeSpace()
    {
        return "+";
    }

    static char
    decodePlus()
    {
        return ' ';
    }
};

class NoPlus {
public:
    static std::string
    encodeSpace()
    {
        return "%20";
    }

    static char
    decodePlus()
    {
        return '+';
    }
};

class RFC2396 {
public:
    static constexpr bool
    match(const char c)
    {
        // clang-format off
        return (
               (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9')
            || (c == '-')
            || (c == '_')
            || (c == '.')
            || (c == '~')
            || (c == '!')
            || (c == '\'')
            || (c == '(')
            || (c == ')')
        );
        // clang-format on
    }
};

class RFC3986 {
public:
    static constexpr bool
    match(const char c)
    {
        // clang-format off
        return (
               (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9')
            || (c == '-')
            || (c == '_')
            || (c == '.')
            || (c == '~')
        );
        // clang-format on
    }
};

template<typename T = detail::RFC3986, typename U = detail::NoPlus>
class Encoder {
public:
    static std::string
    encode(std::string_view s)
    {
        return encode(s.begin(), s.end());
    }

    template<std::forward_iterator It>
    static std::string
    encode(It begin, It end)
    {
        std::ostringstream oss;
        for (It it = begin; it != end; ++it) {
            if (const char c = *it; T::match(c)) {
                oss << c;
            } else if (c == ' ') {
                oss << U::encodeSpace();
            } else {
                oss << '%' << std::hex << std::setw(2) << std::setfill('0') << (c & 0xff);
            }
        }
        return oss.str();
    }

    static std::string
    decode(const std::string_view& s)
    {
        return decode(s.cbegin(), s.cend());
    }

    template<std::forward_iterator It>
    static std::string
    decode(It begin, It end)
    {
        std::ostringstream oss;
        for (It it = begin; it != end; ++it) {
            const char c = *it;
            if (c == '+') {
                oss << U::decodePlus();
            } else if (c == '%') {
                int v{0};
                std::istringstream iss{std::string{++it, it + 2}};
                iss >> std::hex >> v;
                oss << static_cast<char>(v);
                ++it;
            } else {
                oss << c;
            }
        }
        return oss.str();
    }
};

} // namespace detail

namespace uri {

inline std::string
encode(std::string_view input)
{
    return detail::Encoder<detail::RFC3986, detail::NoPlus>::encode(input);
}

inline std::string
decode(std::string_view input)
{
    return detail::Encoder<detail::RFC3986, detail::NoPlus>::decode(input);
}

inline std::string
encode2(std::string_view input)
{
    return detail::Encoder<detail::RFC3986, detail::Plus>::encode(input);
}

inline std::string
decode2(std::string_view input)
{
    return detail::Encoder<detail::RFC3986, detail::Plus>::decode(input);
}

inline static std::string
encodeUriComponent(std::string_view s)
{
    return detail::Encoder<detail::RFC2396, detail::NoPlus>::encode(s);
}

inline static std::string
encodeUriComponent2(std::string_view s)
{
    return detail::Encoder<detail::RFC2396, detail::Plus>::encode(s);
}

} // namespace uri

} // namespace jar
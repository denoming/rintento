#pragma once

#include <boost/property_tree/ptree.hpp>

#include <filesystem>

namespace jar {

class ConfigLoader {
public:
    virtual ~ConfigLoader() = default;

    [[nodiscard]] bool
    load();

    [[nodiscard]] bool
    load(std::istream& stream);

    [[nodiscard]] bool
    load(std::string_view str);

    [[nodiscard]] bool
    load(std::filesystem::path path);

protected:
    virtual void
    doParse(const boost::property_tree::ptree& root)
        = 0;
};

} // namespace jar
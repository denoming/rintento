#pragma once

#include <filesystem>
#include <string_view>

#include <libconfig.h++>

namespace jar {

class ConfigLoader {
public:
    ConfigLoader() = default;

    virtual ~ConfigLoader() = default;

    [[nodiscard]] bool
    load();

    [[nodiscard]] bool
    load(std::string_view str);

    [[nodiscard]] bool
    load(std::filesystem::path file);

protected:
    virtual bool
    doParse(const libconfig::Config& config)
        = 0;
};

} // namespace jar
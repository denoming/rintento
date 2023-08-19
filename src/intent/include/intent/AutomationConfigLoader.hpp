#pragma once

#include "intent/ConfigLoader.hpp"

namespace jar {

class AutomationConfigLoader final : public ConfigLoader {
public:
    AutomationConfigLoader() = default;

private:
    void
    doParse(const boost::property_tree::ptree& root);
};

} // namespace jar
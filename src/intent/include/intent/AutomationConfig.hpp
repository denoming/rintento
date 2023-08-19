#pragma once

#include "intent/ConfigLoader.hpp"

namespace jar {

class AutomationConfig final : public ConfigLoader {
public:
    AutomationConfig() = default;

private:
    void
    doParse(const boost::property_tree::ptree& root);
};

} // namespace jar
#pragma once

#include "common/ConfigLoader.hpp"
#include "intent/Action.hpp"

#include <memory>

namespace jar {

class IAutomationRegistry;

class AutomationConfig final : public ConfigLoader {
public:
    explicit AutomationConfig(std::shared_ptr<IAutomationRegistry> registry);

private:
    void
    doParse(const boost::property_tree::ptree& root) final;

    void
    doParseAutomation(const boost::property_tree::ptree& root);

    Action::List
    doParseActions(const boost::property_tree::ptree& root);

    Action::Ptr
    doParseScriptAction(const boost::property_tree::ptree& root);

private:
    std::shared_ptr<IAutomationRegistry> _registry;
};

} // namespace jar
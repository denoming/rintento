#pragma once

#include "intent/ConfigLoader.hpp"
#include "intent/Action.hpp"

#include <jarvisto/Worker.hpp>

#include <memory>

namespace jar {

class IAutomationRegistry;

class AutomationConfig final : public ConfigLoader {
public:
    AutomationConfig(Worker& worker, IAutomationRegistry& registry);

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
    Worker& _worker;
    IAutomationRegistry& _registry;
};

} // namespace jar
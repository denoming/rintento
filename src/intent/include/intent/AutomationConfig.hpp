#pragma once

#include "intent/ConfigLoader.hpp"
#include "intent/Action.hpp"

#include <jarvisto/Network.hpp>

#include <memory>

namespace jar {

class IAutomationRegistry;

class AutomationConfig final : public ConfigLoader {
public:
    AutomationConfig(io::any_io_executor executor, IAutomationRegistry& registry);

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
    io::any_io_executor _executor;
    IAutomationRegistry& _registry;
};

} // namespace jar
#include "intent/AutomationConfig.hpp"

#include "intent/Automation.hpp"
#include "intent/IAutomationRegistry.hpp"
#include "intent/ScriptAction.hpp"
#include "intent/SequentActionLaunchStrategy.hpp"

#include <jarvisto/Logger.hpp>

namespace jar {

AutomationConfig::AutomationConfig(Worker& worker, IAutomationRegistry& registry)
    : _worker{worker}
    , _registry{registry}
{
}

void
AutomationConfig::doParse(const boost::property_tree::ptree& root)
{
    if (auto automations = root.get_child_optional("automations"); automations) {
        for (auto& [_, automation] : *automations) {
            doParseAutomation(automation);
        }
    }
}

void
AutomationConfig::doParseAutomation(const boost::property_tree::ptree& root)
{
    std::string alias;
    if (auto aliasOpt = root.get_optional<std::string>("alias"); aliasOpt) {
        alias = std::move(*aliasOpt);
    }

    auto intentOpt = root.get_optional<std::string>("intent");
    if (not intentOpt or intentOpt->empty()) {
        LOGE("Mandatory field 'intent' is absent or empty");
        return;
    }
    std::string intent = std::move(*intentOpt);

    if (auto actionsTree = root.get_child_optional("actions"); actionsTree) {
        if (Action::List actions = doParseActions(*actionsTree); not actions.empty()) {
            _registry.add(Automation::create(std::move(alias),
                                             std::move(intent),
                                             std::move(actions),
                                             std::make_shared<SequentActionLaunchStrategy>()));
        }
    } else {
        LOGE("Mandatory field 'actions' is absent or empty");
    }
}

Action::List
AutomationConfig::doParseActions(const boost::property_tree::ptree& root)
{
    Action::List actions;
    for (auto& [_, actionTree] : root) {
        if (auto typeOpt = actionTree.get_optional<std::string>("type"); typeOpt) {
            if (*typeOpt == "script") {
                if (auto action = doParseScriptAction(actionTree); action) {
                    actions.push_back(std::move(action));
                }
                continue;
            }
        }
        /* Parse other action types */
    }
    return actions;
}

Action::Ptr
AutomationConfig::doParseScriptAction(const boost::property_tree::ptree& root)
{
    Action::Ptr action;

    ScriptAction::Environment env;
    if (auto envTree = root.get_child_optional("environment"); envTree) {
        for (const auto& [envName, envValue] : *envTree) {
            if (auto valueOpt = envValue.get_value_optional<std::string>(); valueOpt) {
                env.insert({envName, std::move(*valueOpt)});
            }
        }
    }

    if (auto commandOpt = root.get_optional<std::string>("command"); commandOpt) {
        action = ScriptAction::create(_worker.executor(), std::move(*commandOpt), std::move(env));
    } else {
        LOGE("Mandatory field 'command' is absent");
    }

    return action;
}

} // namespace jar
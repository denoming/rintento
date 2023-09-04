#include "intent/AutomationConfig.hpp"

#include "intent/Automation.hpp"
#include "intent/IAutomationRegistry.hpp"
#include "intent/ScriptAction.hpp"
#include "intent/SequentLaunchStrategy.hpp"

#include <jarvisto/Logger.hpp>

#include <filesystem>

namespace fs = std::filesystem;

namespace jar {

AutomationConfig::AutomationConfig(std::shared_ptr<IAutomationRegistry> registry)
    : _registry{std::move(registry)}
{
    BOOST_ASSERT(_registry);
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
            _registry->add(Automation::create(std::move(alias),
                                              std::move(intent),
                                              std::move(actions),
                                              std::make_shared<SequentLaunchStrategy>()));
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
    fs::path execPath;
    if (auto execOpt = root.get_optional<std::string>("exec"); execOpt) {
        execPath = std::move(*execOpt);
    } else {
        LOGE("Mandatory field 'exec' is absent");
        return {};
    }

    ScriptAction::Args args;
    if (auto argsTree = root.get_child_optional("args"); argsTree) {
        for (const auto& [_, valueTree] : *argsTree) {
            if (auto valueOpt = valueTree.get_value_optional<std::string>(); valueOpt) {
                args.push_back(std::move(*valueOpt));
            }
        }
    }

    fs::path homePath;
    if (auto homeOpt = root.get_optional<std::string>("home"); homeOpt) {
        homePath = std::move(*homeOpt);
    }

    ScriptAction::Environment env;
    if (auto envTree = root.get_child_optional("environment"); envTree) {
        for (const auto& [envName, envValue] : *envTree) {
            if (auto valueOpt = envValue.get_value_optional<std::string>(); valueOpt) {
                env.insert({envName, std::move(*valueOpt)});
            }
        }
    }

    bool inheritParentEnv{false};
    if (auto inheritOpt = root.get_optional<bool>("inheritParentEnv"); inheritOpt) {
        inheritParentEnv = *inheritOpt;
    }

    ScriptAction::Timeout timeout{ScriptAction::kDefaultTimeout};
    if (auto timeoutOpt = root.get_optional<int64_t>("timeout"); timeoutOpt) {
        if (*timeoutOpt > 0) {
            timeout = ScriptAction::Timeout{*timeoutOpt};
        } else {
            LOGE("Invalid value for timeout field: {}", *timeoutOpt);
        }
    }

    return ScriptAction::create(std::move(execPath),
                                std::move(args),
                                std::move(homePath),
                                std::move(env),
                                inheritParentEnv,
                                timeout);
}

} // namespace jar
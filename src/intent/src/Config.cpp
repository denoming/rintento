#include "intent/Config.hpp"

#include "intent/Automation.hpp"
#include "intent/IAutomationRegistry.hpp"
#include "intent/MqttAction.hpp"
#include "intent/ScriptAction.hpp"
#include "intent/SequentLaunchStrategy.hpp"
#include "rintento/Options.hpp"

#include <boost/assert.hpp>

#include <jarvisto/Logger.hpp>

namespace fs = std::filesystem;

namespace jar {

namespace {

Action::Ptr
parseMqttAction(const libconfig::Setting& root)
{
    std::string topic;
    if (not root.lookupValue("topic", topic)) {
        LOGE("No 'topic' field");
        return {};
    }
    std::string value;
    if (not root.lookupValue("value", value)) {
        LOGE("No 'value' field");
        return {};
    }
    std::string host;
    if (not root.lookupValue("host", host)) {
        LOGE("No 'host' field");
        return {};
    }
    uint32_t port{MqttAction::kDefaultPort};
    root.lookupValue("port", port);

    auto action = MqttAction::create(std::move(topic), std::move(value), std::move(host), port);
    BOOST_ASSERT(action);

    std::string user, pass;
    if (root.lookupValue("user", user)) {
        if (root.lookupValue("pass", pass)) {
            action->credentials(std::move(user), std::move(pass));
        }
    }

    return action;
}

Action::Ptr
parseScriptAction(const libconfig::Setting& root)
{
    std::string exec;
    if (not root.lookupValue("exec", exec)) {
        LOGE("No 'exec' field");
        return {};
    }

    ScriptAction::Args args;
    try {
        for (const auto& e : root["args"]) {
            args.push_back(e);
        }
    } catch (const libconfig::SettingTypeException& e) {
        LOGE("Wrong element type: {}", e.what());
    } catch (const libconfig::SettingNotFoundException& e) {
        // Ignore
    }

    std::string home;
    std::ignore = root.lookupValue("home", home);

    ScriptAction::Environment env;
    try {
        for (const auto& e : root["env"]) {
            env.insert(std::make_pair<std::string, std::string>(e.getName(), e));
        }
    } catch (const libconfig::SettingTypeException& e) {
        LOGE("Wrong element type: {}", e.what());
    } catch (const libconfig::SettingNotFoundException& e) {
        // Ignore
    }

    bool inheritParentEnv{false};
    std::ignore = root.lookupValue("inheritParentEnv", inheritParentEnv);

    uint32_t timeout = ScriptAction::kDefaultTimeout.count();
    if (root.lookupValue("timeout", timeout) and timeout <= 0) {
        LOGE("Invalid value for timeout field: {}", timeout);
        timeout = ScriptAction::kDefaultTimeout.count();
    }

    return ScriptAction::create(std::move(exec),
                                std::move(args),
                                std::move(home),
                                std::move(env),
                                inheritParentEnv,
                                ScriptAction::Timeout{timeout});
}

Action::List
parseActions(const libconfig::Setting& root)
{
    Action::List actions;
    for (int i = 0; i < root.getLength(); ++i) {
        std::string type;
        if (not root[i].lookupValue("type", type)) {
            LOGE("No 'type' field");
        } else {
            if (type == "script") {
                if (auto action = parseScriptAction(root[i]); action) {
                    actions.push_back(std::move(action));
                }
                continue;
            }
            if (type == "mqtt") {
                if (auto action = parseMqttAction(root[i]); action) {
                    actions.push_back(std::move(action));
                }
                continue;
            }
            LOGW("Not supported 'type' field value: {}", type);
        }
    }
    return actions;
}

} // namespace

Config::Config(std::shared_ptr<IAutomationRegistry> registry)
    : _registry{std::move(registry)}
{
    BOOST_ASSERT(_registry);
}

uint32_t
Config::serverPort() const
{
    return _serverPort;
}

uint32_t
Config::serverThreads() const
{
    return _serverThreads;
}

std::optional<std::string>
Config::witRemoteHost() const
{
    return _witRemoteHost;
}

std::optional<std::string>
Config::witRemotePort() const
{
    return _witRemotePort;
}

std::optional<std::string>
Config::witRemoteAuth() const
{
    return _witRemoteAuth;
}

bool
Config::doParse(const libconfig::Config& config)
{
    try {
        std::ignore = config.lookupValue("server.port", _serverPort);
        std::ignore = config.lookupValue("server.threads", _serverThreads);

#ifdef ENABLE_WIT_SUPPORT
        std::string witRemoteHost;
        if (config.lookupValue("wit.remote.host", witRemoteHost)) {
            _witRemoteHost = std::move(witRemoteHost);
        }
        std::string witRemotePort;
        if (config.lookupValue("wit.remote.port", witRemotePort)) {
            _witRemotePort = std::move(witRemotePort);
        }
        std::string witRemoteAuth;
        if (config.lookupValue("wit.remote.auth", witRemoteAuth)) {
            _witRemoteAuth = std::move(witRemoteAuth);
        }
#endif

        static const char* kAutomationsKey{"automations"};
        if (config.exists(kAutomationsKey)) {
            doParseAutomations(config.lookup(kAutomationsKey));
        }
    } catch (...) {
        // Suppress any exceptions
    }
    return true;
}

void
Config::doParseAutomations(const libconfig::Setting& root)
{
    for (int i = 0; i < root.getLength(); ++i) {
        const auto& automation = root[i];

        std::string alias;
        std::ignore = automation.lookupValue("alias", alias);

        std::string intent;
        if (not automation.lookupValue("intent", intent)) {
            LOGE("No 'intent' field");
            continue;
        }

        if (auto actions = parseActions(automation.lookup("actions")); not actions.empty()) {
            BOOST_ASSERT(_registry);
            _registry->add(Automation::create(std::move(alias),
                                              std::move(intent),
                                              std::move(actions),
                                              std::make_shared<SequentLaunchStrategy>()));
        }
    }
}

} // namespace jar
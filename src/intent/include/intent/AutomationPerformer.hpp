#pragma once

#include "intent/WitTypes.hpp"

#include <jarvisto/Network.hpp>

#include <map>
#include <memory>
#include <string>

namespace jar {

class Automation;
class AutomationRegistry;

class AutomationPerformer : public std::enable_shared_from_this<AutomationPerformer> {
public:
    explicit AutomationPerformer(AutomationRegistry& registry);

    void
    perform(const wit::Utterances& utterances);

    void
    perform(const std::string& intent);

private:
    void
    onAutomationDone(const std::string& id, const std::string& alias, std::error_code ec);

private:
    AutomationRegistry& _registry;
    std::map<std::string, std::shared_ptr<Automation>> _runningList;
};

} // namespace jar
#pragma once

#include "intent/WitTypes.hpp"

#include <jarvisto/Network.hpp>

#include <map>
#include <memory>
#include <string>

namespace jar {

class Automation;
class AutomationRegistry;

class AutomationExecutor : public std::enable_shared_from_this<AutomationExecutor> {
public:
    explicit AutomationExecutor(std::shared_ptr<AutomationRegistry> registry);

    void
    execute(const wit::Utterances& utterances);

    void
    execute(const std::string& intent);

private:
    void
    onAutomationDone(const std::string& id, const std::string& alias, std::error_code ec);

private:
    std::shared_ptr<AutomationRegistry> _registry;
    std::map<std::string, std::shared_ptr<Automation>> _runningList;
};

} // namespace jar
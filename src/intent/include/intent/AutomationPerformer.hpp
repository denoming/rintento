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
    using Ptr = std::shared_ptr<AutomationPerformer>;

    static Ptr
    create(io::any_io_executor executor, std::shared_ptr<AutomationRegistry> registry);

    void
    perform(const wit::Utterances& utterances);

    void
    perform(const std::string& intent);

private:
    explicit AutomationPerformer(io::any_io_executor executor,
                                 std::shared_ptr<AutomationRegistry> registry);

private:
    void
    onAutomationDone(const std::string& id, const std::string& alias, std::error_code ec);

private:
    io::any_io_executor _executor;
    std::shared_ptr<AutomationRegistry> _registry;
    std::map<std::string, std::shared_ptr<Automation>> _runningList;
};

} // namespace jar
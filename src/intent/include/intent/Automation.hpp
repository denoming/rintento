#pragma once

#include "intent/Action.hpp"
#include "intent/DeferredJob.hpp"
#include "intent/LaunchStrategy.hpp"

#include <jarvisto/Network.hpp>

#include <functional>
#include <memory>
#include <string>
#include <system_error>

namespace jar {

class Automation final : public std::enable_shared_from_this<Automation>, public DeferredJob {
public:
    using Ptr = std::shared_ptr<Automation>;

    static Ptr
    create(std::string alias,
           std::string intent,
           Action::List actions,
           LaunchStrategy::Ptr launchStrategy);

    [[nodiscard]] const std::string&
    id() const;

    [[nodiscard]] const std::string&
    alias() const;

    [[nodiscard]] const std::string&
    intent() const;

    Ptr
    clone();

    void
    execute(io::any_io_executor executor);

private:
    Automation(std::string id,
               std::string alias,
               std::string intent,
               Action::List actions,
               LaunchStrategy::Ptr launchStrategy);

    void
    onExecuteDone(std::error_code ec);

private:
    std::string _id;
    std::string _alias;
    std::string _intent;
    Action::List _actions;
    LaunchStrategy::Ptr _launcher;
};

} // namespace jar

#include "intent/ScriptAction.hpp"

namespace jar {

Action::Ptr
ScriptAction::create(io::any_io_executor executor)
{
    return Action::Ptr{new ScriptAction{std::move(executor)}};
}

ScriptAction::ScriptAction(io::any_io_executor executor)
    : _executor{std::move(executor)}
{
}

ScriptAction::Ptr
ScriptAction::clone() const
{
    return Action::Ptr{new ScriptAction{*this}};
}

void
ScriptAction::execute()
{
    io::post(_executor, [weakSelf = weak_from_this()]() {
        if (auto self = weakSelf.lock()) {
            self->runCmd();
        }
    });
}

void
ScriptAction::runCmd()
{
    /* ToDo: run script or any other program */
}

} // namespace jar

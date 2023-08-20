#include "intent/ScriptAction.hpp"

namespace jar {

Action::Ptr
ScriptAction::create(io::any_io_executor executor, std::string cmd, Environment env)
{
    return Action::Ptr{new ScriptAction{std::move(executor), std::move(cmd), std::move(env)}};
}

ScriptAction::ScriptAction(io::any_io_executor executor, std::string cmd, Environment env)
    : _executor{std::move(executor)}
    , _cmd{std::move(cmd)}
    , _env{std::move(env)}
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
            self->run();
        }
    });
}

void
ScriptAction::run()
{
    /* ToDo: run script or any other program */
}

} // namespace jar

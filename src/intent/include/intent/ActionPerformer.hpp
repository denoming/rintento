#pragma once

#include "intent/Types.hpp"

#include <sigc++/trackable.h>

#include <memory>
#include <queue>

namespace jar {

class Action;
class ActionRegistry;

class ActionPerformer : public sigc::trackable,
                        public std::enable_shared_from_this<ActionPerformer> {
public:
    static std::shared_ptr<ActionPerformer>
    create(ActionRegistry& registry);

    void
    perform(Utterances utterances);

private:
    ActionPerformer(ActionRegistry& registry);

    void onActionDone(std::error_code);

private:
    ActionRegistry& _registry;

    std::queue<std::shared_ptr<Action>> _pendingActions;
};

} // namespace jar
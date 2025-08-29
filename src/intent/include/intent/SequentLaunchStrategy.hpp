// Copyright 2025 Denys Asauliak
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "intent/LaunchStrategy.hpp"

#include <jarvisto/network/Asio.hpp>

#include <memory>

namespace jar {

class SequentLaunchStrategy final : public std::enable_shared_from_this<SequentLaunchStrategy>,
                                    public LaunchStrategy {
public:
    SequentLaunchStrategy() = default;

    [[nodiscard]] Ptr
    clone() const final;

    void
    launch(io::any_io_executor executor, Action::List actions) final;

private:
    [[nodiscard]] Action::Ptr
    currentAction() const;

    std::size_t
    currentActionIndex() const;

    [[nodiscard]] bool
    hasNextAction() const;

    void
    executeNextAction();

    void
    onActionDone(std::error_code ec);

private:
    io::any_io_executor _executor;
    std::size_t _currIndex{};
    std::size_t _nextIndex{};
    Action::List _actions;
};

} // namespace jar
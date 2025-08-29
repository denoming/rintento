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

#include "intent/DeferredJob.hpp"

#include <jarvisto/network/Asio.hpp>

#include <memory>
#include <vector>

namespace jar {

class Action : public DeferredJob {
public:
    using Ptr = std::shared_ptr<Action>;
    using List = std::vector<Ptr>;

    Action() = default;

    virtual ~Action() = default;

    [[nodiscard]] virtual Ptr
    clone() const
        = 0;

    virtual void
    execute(io::any_io_executor executor)
        = 0;
};

} // namespace jar
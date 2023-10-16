#pragma once

#include "common/Types.hpp"

#include <jarvisto/Asio.hpp>
#include <jarvisto/Cancellable.hpp>

namespace jar {

class Recognition : public Cancellable {
public:
    virtual ~Recognition() = default;

    virtual io::awaitable<RecognitionResult>
    run() = 0;
};

} // namespace jar

#pragma once

#include "common/Types.hpp"

#include <jarvisto/network/Asio.hpp>
#include <jarvisto/network/Cancellable.hpp>

namespace jar {

class Recognition : public Cancellable {
public:
    virtual ~Recognition() = default;

    virtual io::awaitable<RecognitionResult>
    run() = 0;
};

} // namespace jar

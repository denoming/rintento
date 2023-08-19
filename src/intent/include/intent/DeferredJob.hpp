#pragma once

#include <functional>
#include <system_error>

namespace jar {

class DeferredJob {
public:
    using OnDone = void(std::error_code);

    DeferredJob() = default;

    void
    onDone(std::function<OnDone> callback);

protected:
    void
    complete(std::error_code ec = {});

private:
    std::function<OnDone> _onDone;
};

} // namespace jar
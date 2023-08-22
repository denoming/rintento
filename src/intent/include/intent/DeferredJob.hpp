#pragma once

#include <functional>
#include <system_error>

namespace jar {

class DeferredJob {
public:
    using OnComplete = void(std::error_code);

    DeferredJob() = default;

    void
    onComplete(std::function<OnComplete> callback);

protected:
    void
    complete(std::error_code ec = {});

private:
    std::function<OnComplete> _onComplete;
};

} // namespace jar
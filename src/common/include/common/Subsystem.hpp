#pragma once

#include <memory>

namespace jar {

class Application;

class Subsystem {
public:
    using Ptr = std::unique_ptr<Subsystem>;

    virtual ~Subsystem() = default;

    virtual const char*
    name() const
        = 0;

protected:
    friend class Application;

    virtual void
    initialize(Application& application);

    virtual void
    setUp(Application& application);

    virtual void
    tearDown();

    virtual void
    finalize();
};

} // namespace jar
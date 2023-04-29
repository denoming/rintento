#pragma once

#include "intent/Action.hpp"

namespace jar {

class DateTimeAction : public Action {
public:
    DateTimeAction(std::string intent, Entities entities);

protected:
    [[nodiscard]] bool
    hasTimestamps() const;

    const Timestamp&
    timestampFrom() const;

    const Timestamp&
    timestampTo() const;

private:
    void
    retrieveTimestamps();

private:
    Timestamp _tsFrom;
    Timestamp _tsTo;
};

} // namespace jar
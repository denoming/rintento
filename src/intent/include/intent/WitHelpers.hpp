#pragma once

#include <jarvis/weather/CustomData.hpp>

#include "intent/WitTypes.hpp"

#include <chrono>

namespace jar::wit {

/**
 * Predicate to filter custom data by given timestamp range interval with particular duration.
 *
 * Takes into account \b dt and \b duration fields of custom data to calculate intersection with
 * custom data timeline.
 */
class DateTimePredicate {
public:
    DateTimePredicate() = default;

    DateTimePredicate(Timestamp from, Timestamp to);

    /**
     * Checks if custom data intersects with given timestamp range
     * @param data Given custom data intersect
     * @return \c true iff custom data intersects with timestamp range
     */
    bool
    operator()(const CustomData& data) const;

private:
    Timestamp _from;
    Timestamp _to;
};

/**
 * Getter for retrieving particular entity type.
 *
 * @tparam T The type of entity (e.g. DataTimeEntity)
 */
template<typename T>
class EntityGetter {
public:
    EntityGetter(const Entities& entities)
        : _entities{entities}
    {
    }

    bool
    has()
    {
        return _entities.contains(T::key());
    }

    const T&
    get()
    {
        for (auto&& entity : _entities.at(T::key())) {
            if (std::holds_alternative<T>(entity)) {
                return std::get<T>(entity);
            }
        }
        throw std::out_of_range{"Item not found"};
    }

private:
    const Entities& _entities;
};



} // namespace jar::wit
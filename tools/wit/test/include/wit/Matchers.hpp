#pragma once

#include "common/Types.hpp"

#include <boost/system/system_error.hpp>

#include <gmock/gmock.h>

namespace jar {

inline testing::Matcher<RecognitionResult>
understoodIntent(const std::string& intent)
{
    return AllOf(Field(&RecognitionResult::isUnderstood, testing::IsTrue()),
                 testing::Field(&RecognitionResult::intent, intent));
}

inline testing::Matcher<RecognitionResult>
notUnderstoodIntent()
{
    return Field(&RecognitionResult::isUnderstood, testing::IsFalse());
}

inline auto
exceptionContainsError(testing::Matcher<int> matcher)
{
    return [matcher = std::move(matcher)](const std::exception_ptr& eptr) {
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch (const boost::system::system_error& e) {
            return Matches(matcher)(e.code().value());
        } catch (const std::exception& e) {
            /* Unexpected exception */
        }
        return false;
    };
}

} // namespace jar

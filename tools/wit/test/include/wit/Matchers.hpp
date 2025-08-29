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

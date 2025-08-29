# Copyright 2025 Denys Asauliak
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/modules")

if (EXISTS $ENV{HOME}/.local)
    list(APPEND CMAKE_PREFIX_PATH $ENV{HOME}/.local)
endif()

if (DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Toolchain file ${CMAKE_TOOLCHAIN_FILE}")
endif()

include(BuildType)
include(BuildLocation)
include(BuildOptions)

include(EnableCcache)

if(ENABLE_CODE_FORMAT)
    # Include all dirs with source code
    set(CLANG_FORMAT_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/tools
        ${PROJECT_SOURCE_DIR}/test
    )
    include(CodeFormat)
endif()

if (ENABLE_TESTS)
    enable_testing()
    include(AddGoogleTest)
    message(VERBOSE "Building with tests")
else()
    message(VERBOSE "Building without tests")
endif()

include(AddBoost)
include(AddOpenSsl)
include(AddSpdLog)
include(AddJarvisto)
if(ENABLE_CLI OR ENABLE_TESTS)
    include(AddSndFile)
endif()
include(AddLibConfig)

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

cmake_minimum_required(VERSION 3.25)

include(cmake/ProjectOptions.cmake)

project(rintento
    VERSION 1.0.2
    DESCRIPTION "Rintento Speech Project"
)

include(cmake/ProjectConfigs.cmake)
include(cmake/ProjectInstall.cmake)

add_subdirectory(config)
add_subdirectory(src)
if(ENABLE_TESTS)
    add_subdirectory(test)
endif()
add_subdirectory(tools)

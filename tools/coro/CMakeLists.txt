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

set(TARGET rintento-coro)

add_library(${TARGET} INTERFACE)
add_library(Rintento::Coro ALIAS ${TARGET})

target_include_directories(${TARGET}
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    INTERFACE Boost::headers
)

target_compile_features(${TARGET} INTERFACE cxx_std_23)

if(ENABLE_TESTS)
    add_subdirectory(test)
endif()
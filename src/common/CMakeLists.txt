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

set(TARGET rintento-common)

add_library(${TARGET} STATIC)
add_library(Rintento::Common ALIAS ${TARGET})

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC Jarvisto::Core
    PUBLIC Jarvisto::Network
           Boost::headers
           PkgConfig::LibConfig
)

target_sources(${TARGET}
    PRIVATE src/ConfigLoader.cpp
            src/ServiceLogger.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

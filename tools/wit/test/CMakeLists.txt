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

set(TARGET rintento-wit-test)

add_executable(${TARGET} "")

target_sources(${TARGET}
    PRIVATE src/IntentParserTest.cpp
            src/MessageRecognitionTest.cpp
            src/SpeechRecognitionTest.cpp
            src/ConfigTest.cpp
            src/UtilsTest.cpp
)

target_include_directories(${TARGET}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PRIVATE Rintento::Wit
            Rintento::Test
            PkgConfig::SndFile
)

install(
    TARGETS ${TARGET}
    COMPONENT RintentoExecutorRuntime
)

if (NOT CMAKE_CROSSCOMPILING)
    gtest_discover_tests(${TARGET}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        PROPERTIES LABELS "Unit"
    )
endif()

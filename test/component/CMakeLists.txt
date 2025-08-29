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

set(Python_FIND_VIRTUALENV FIRST)

find_package(Python COMPONENTS Interpreter)

if (NOT Python_FOUND)
    message(FATAL_ERROR "Python interpreter is not found")
endif()

set(PyTestExec ${Python_EXECUTABLE} -m pytest -p no:cacheprovider)
set(PyTestMain ${PyTestExec} --exec-path=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rintento)

add_test(
    NAME TestMessageRecognition
    COMMAND ${PyTestMain}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/test_message_recognition.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

set_tests_properties(TestMessageRecognition PROPERTIES LABELS "Component")

add_test(
    NAME TestSpeechRecognition
    COMMAND ${PyTestMain}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/test_speech_recognition.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

set_tests_properties(TestSpeechRecognition PROPERTIES LABELS "Component")

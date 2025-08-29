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

#[=======================================================================[.rst:
FindClangFormat
-------

Finds the ClangFormat Tool.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``ClangFormat_FOUND``
  True if the system has the cppcheck tool.
``ClangFormat_VERSION``
  Version of cppcheck tool.

#]=======================================================================]

if(CLANG_FORMAT_ROOT_DIR)
    find_program(
        CLANG_FORMAT_BIN
        NAMES clang-format-16 clang-format-15 clang-format
        PATHS "${CLANG_FORMAT_ROOT_DIR}"
        NO_DEFAULT_PATH
    )
endif()

if(NOT CLANG_FORMAT_BIN)
    find_program(
        CLANG_FORMAT_BIN NAMES clang-format-16 clang-format-15 clang-format
    )
endif()

if(CLANG_FORMAT_BIN)
    execute_process(
        COMMAND ${CLANG_FORMAT_BIN} --version
        OUTPUT_VARIABLE CLANG_FORMAT_VERSION_OUTPUT
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(CLANG_FORMAT_VERSION_OUTPUT MATCHES "^clang-format version .*")
        string(REGEX
               REPLACE "clang-format version ([.0-9]+).*" "\\1"
                       ClangFormat_VERSION "${CLANG_FORMAT_VERSION_OUTPUT}"
        )
    else()
        message(FATAL "Couldn't recognize ${CLANG_FORMAT_BIN} version")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    ClangFormat REQUIRED_VARS CLANG_FORMAT_BIN VERSION_VAR ClangFormat_VERSION
)

mark_as_advanced(CLANG_FORMAT_BIN CLANG_FORMAT_VERSION_OUTPUT)

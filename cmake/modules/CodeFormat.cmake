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
CodeFormat
-------

Add code format target.

Custom Targets
^^^^^^^^^^^^^^^^

This module provides the following custom targets, if found:

``codeformat``
  The ClangFormat custom target to format code according to format file.

``formatcheck``
  The ClangFormat custom target to check code according to format file.

Expected that format file placed at the root of project.

Config Variables
^^^^^^^^^^^^^^^^

``CLANG_FORMAT_INCLUDE_DIRS``
  Specify dirs explicitly where to search files. By default: <empty>

``CLANG_FORMAT_FILE_EXTENSIONS``
  Specify file extension to search for. By default: *.cpp *.h *.cxx *.hxx *.hpp *.cc *.ipp

``CLANG_FORMAT_EXCLUDE_PATTERNS``
  Specify exclude patterns. By default: "/CMakeFiles/" "cmake"

#]=======================================================================]

if(TARGET codeformat)
    return()
endif()

find_package(ClangFormat)

if(ClangFormat_FOUND)
    set(CLANG_FORMAT_FILE_EXTENSIONS
        ${CLANG_FORMAT_FILE_EXTENSIONS}
        *.cpp
        *.hpp
        *.cxx
        *.h
        *.hxx
        *.cc
        *.ipp
    )

    if(DEFINED CLANG_FORMAT_INCLUDE_DIRS)
        set(CLANG_FORMAT_GLOB_EXPRESSIONS "")
        foreach(CLANG_FORMAT_INCLUDE_DIR ${CLANG_FORMAT_INCLUDE_DIRS})
            foreach(CLANG_FORMAT_CXX_FILE_EXTENSION ${CLANG_FORMAT_FILE_EXTENSIONS})
                list(APPEND CLANG_FORMAT_GLOB_EXPRESSIONS "${CLANG_FORMAT_INCLUDE_DIR}/${CLANG_FORMAT_CXX_FILE_EXTENSION}")
            endforeach()
        endforeach()
    else()
        set(CLANG_FORMAT_GLOB_EXPRESSIONS ${CLANG_FORMAT_FILE_EXTENSIONS})
    endif()

    # Create the list with all source files
    file(GLOB_RECURSE ALL_SOURCE_FILES ${CLANG_FORMAT_GLOB_EXPRESSIONS})

    set(CLANG_FORMAT_EXCLUDE_PATTERNS ${CLANG_FORMAT_EXCLUDE_PATTERNS}
                                      "/CMakeFiles/" "cmake"
    )

    foreach(SOURCE_FILE ${ALL_SOURCE_FILES})
        foreach(EXCLUDE_PATTERN ${CLANG_FORMAT_EXCLUDE_PATTERNS})
            string(FIND ${SOURCE_FILE} ${EXCLUDE_PATTERN} EXCLUDE_FOUND)
            if(NOT ${EXCLUDE_FOUND} EQUAL -1)
                list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
            endif()
        endforeach()
    endforeach()

    add_custom_target(
        codeformat
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Running clang-format to reformat files"
        VERBATIM
        COMMAND ${CLANG_FORMAT_BIN} -style=file -i ${ALL_SOURCE_FILES}
    )

    add_custom_target(
        formatcheck
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Running clang-format to check files"
        VERBATIM
        COMMAND
            bash -c
            "echo '${ALL_SOURCE_FILES}' | xargs -n 1 -d ';' -I {} bash -c 'diff -u <(cat {}) <(${CLANG_FORMAT_BIN} {})'"
    )
endif()

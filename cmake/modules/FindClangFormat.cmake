# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

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

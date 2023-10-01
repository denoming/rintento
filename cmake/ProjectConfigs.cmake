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

include(EnableCcache)

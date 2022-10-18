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

if (JARVIS_ENABLE_TESTS)
    enable_testing()
    include(AddGoogleTest)
    message(VERBOSE "Building with tests")
else()
    message(VERBOSE "Building without tests")
endif()

include(AddBoost)
include(AddOpenSsl)
include(AddSpdLog)
include(AddBoostUrl)

include(EnableCcache)

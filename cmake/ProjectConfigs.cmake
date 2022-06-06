list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/modules")

if (EXISTS $ENV{HOME}/.local)
    list(APPEND CMAKE_PREFIX_PATH $ENV{HOME}/.local)
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
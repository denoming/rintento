include(FeatureSummary)

option(ENABLE_CODE_FORMAT "Enable code formatting" ON)
add_feature_info(
    ENABLE_CODE_FORMAT ENABLE_CODE_FORMAT "Enable code formatting support"
)

option(ENABLE_TESTS "Enable testing" ON)
if(ENABLE_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()
add_feature_info(
    ENABLE_TESTS ENABLE_TESTS "Build project with tests"
)

option(ENABLE_CLI "Enable CLI" OFF)
if(ENABLE_CLI)
    list(APPEND VCPKG_MANIFEST_FEATURES "cli")
endif()
add_feature_info(
    ENABLE_CLI ENABLE_CLI "Build project with CLI"
)

option(ENABLE_WIT_SUPPORT "Enable wit.ai support" ON)
add_feature_info(
    ENABLE_WIT_SUPPORT ENABLE_WIT_SUPPORT "Build project with wit.ai support"
)

feature_summary(WHAT ALL)

include(FeatureSummary)

option(ENABLE_TESTS "Enable testing" ON)
add_feature_info(
    ENABLE_TESTS ENABLE_TESTS "Build project with tests"
)

option(ENABLE_DLT "Enable DLT logging" OFF)
add_feature_info(
    ENABLE_DLT ENABLE_DLT "Build project with DLT logging"
)

option(ENABLE_WIT_SUPPORT "Enable wit.ai support" ON)
add_feature_info(
    ENABLE_WIT_SUPPORT ENABLE_WIT_SUPPORT "Build project with wit.ai support"
)

feature_summary(WHAT ALL)

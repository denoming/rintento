include(FeatureSummary)

option(ENABLE_TESTS "Enable testing" ON)
add_feature_info(
    ENABLE_TESTS ENABLE_TESTS "Build project with tests"
)

option(ENABLE_DLT "Enable DLT logging" OFF)
add_feature_info(
    ENABLE_DLT ENABLE_DLT "Build project with DLT logging"
)

feature_summary(WHAT ALL)

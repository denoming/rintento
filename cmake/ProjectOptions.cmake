include(FeatureSummary)

option(TEMPLATE01_ENABLE_TESTS "Enable testing" ON)
add_feature_info(
    ENABLE_TESTS TEMPLATE01_ENABLE_TESTS "Build project with tests"
)

feature_summary(WHAT ALL)

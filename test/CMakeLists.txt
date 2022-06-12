set(TARGET jarvis-unit-test)

add_executable(${TARGET} "")

set_target_properties(${TARGET}
    PROPERTIES
        OUTPUT_NAME jarvis-unit-test
)

target_sources(${TARGET}
    PRIVATE src/DummyTest.cpp
)

target_link_libraries(
    ${TARGET}
    PRIVATE GTest::gtest_main GTest::gmock_main
)

if (NOT CMAKE_CROSSCOMPILING)
    gtest_discover_tests(${TARGET})
endif()

set(TARGET "rintento-coro-test")

add_executable(${TARGET} "")

target_sources(${TARGET}
    PRIVATE
        src/ConditionTest.cpp
        src/BoundedChannelTest.cpp
)

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PRIVATE Rintento::Coro
            GTest::gtest_main
            GTest::gmock_main
)

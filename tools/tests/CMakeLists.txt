set(TARGET jarvis-tests)

add_library(${TARGET} STATIC)
add_library(Jarvis::Tests ALIAS ${TARGET})

target_link_libraries(${TARGET}
    PUBLIC Jarvis::Common
    PUBLIC GTest::gtest GTest::gmock
)

target_sources(${TARGET}
    PRIVATE src/TestMain.cpp
        src/TestWorker.cpp
)

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)
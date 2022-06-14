set(TARGET jarvis-tests)

add_library(${TARGET} STATIC)
add_library(Jarvis::Tests ALIAS ${TARGET})

target_link_libraries(${TARGET}
    PUBLIC Jarvis::Common
    PUBLIC GTest::gtest GTest::gmock
)

target_sources(${TARGET}
    PRIVATE src/TestMain.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)
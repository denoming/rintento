set(TARGET rintento-test-common)

add_library(${TARGET} STATIC "")
add_library(Rintento::Test ALIAS ${TARGET})

target_sources(${TARGET}
    PRIVATE src/TestMain.cpp
            src/TestWaiter.cpp
            src/Utils.cpp
)

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC GTest::gtest
           GTest::gmock
           Rintento::Intent
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

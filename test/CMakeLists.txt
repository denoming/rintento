set(TARGET rintento-test)

add_library(${TARGET} STATIC "")
add_library(Rintento::Test ALIAS ${TARGET})

target_sources(${TARGET}
    PRIVATE src/TestMain.cpp
)

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC GTest::gtest
           GTest::gmock
           Jarvisto::Jarvisto
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

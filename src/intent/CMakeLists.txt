set(TARGET jarvis-intent)

add_library(${TARGET} STATIC)
add_library(Jarvis::Intent ALIAS ${TARGET})

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC Boost::headers
)

target_sources(${TARGET}
    PRIVATE src/IntentRecognizer.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

target_compile_definitions(${TARGET}
    PRIVATE BOOST_BEAST_USE_STD_STRING_VIEW
)
set(TARGET rintento-process)

add_library(${TARGET} "")
add_library(Rintento::Process ALIAS ${TARGET})

target_sources(${TARGET}
    PRIVATE
        src/Process.cpp
)

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC Boost::headers
)

target_compile_definitions(${TARGET}
    PUBLIC BOOST_PROCESS_USE_STD_FS=1
           BOOST_PROCESS_NO_DEPRECATED=1
    PRIVATE BOOST_PROCESS_V2_SEPARATE_COMPILATION=1
)

target_compile_features(${TARGET} PRIVATE cxx_std_23)
set(TARGET rintento-coro)

add_library(${TARGET} INTERFACE)
add_library(Rintento::Coro ALIAS ${TARGET})

target_include_directories(${TARGET}
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    INTERFACE Boost::headers
)

target_compile_features(${TARGET} INTERFACE cxx_std_23)

if(ENABLE_TESTS)
    add_subdirectory(test)
endif()
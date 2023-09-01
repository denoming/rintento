set(TARGET rintento-coro)

add_library(${TARGET} "")
add_library(Rintento::Coro ALIAS ${TARGET})

target_sources(${TARGET}
    PRIVATE
        src/Event.cpp
)

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC Boost::headers
)

target_compile_features(${TARGET} PRIVATE cxx_std_23)
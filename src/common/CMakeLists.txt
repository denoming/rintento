set(TARGET rintento-common)

add_library(${TARGET} STATIC)
add_library(Rintento::Common ALIAS ${TARGET})

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC Jarvisto::Core
    PUBLIC Jarvisto::Network
           Boost::headers
           PkgConfig::LibConfig
)

target_sources(${TARGET}
    PRIVATE src/ConfigLoader.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

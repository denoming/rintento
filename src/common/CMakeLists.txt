set(TARGET rintento-common)

add_library(${TARGET} STATIC)
add_library(Rintento::Common ALIAS ${TARGET})

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PRIVATE Boost::headers
            Jarvisto::Jarvisto
)

target_sources(${TARGET}
    PRIVATE src/Config.cpp
            src/Utils.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

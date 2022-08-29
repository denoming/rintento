include(ExternalProject)

set(installDir ${CMAKE_CURRENT_BINARY_DIR}/external/url)

ExternalProject_Add(boost-url
    GIT_REPOSITORY https://github.com/CPPAlliance/url.git
    GIT_TAG ef2db3c1e52474b881812da92ae495cd936ed150
    INSTALL_DIR ${installDir}
    CMAKE_ARGS -DBOOST_URL_BUILD_TESTS=OFF
               -DBOOST_URL_BUILD_EXAMPLES=OFF
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    EXCLUDE_FROM_ALL TRUE
)

add_library(Boost::url STATIC IMPORTED)

set_target_properties(Boost::url PROPERTIES IMPORTED_LOCATION ${installDir}/lib/libboost_url.a)

target_include_directories(Boost::url INTERFACE ${installDir}/include)

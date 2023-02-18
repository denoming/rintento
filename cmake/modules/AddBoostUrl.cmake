include(FetchContent)
FetchContent_Declare(boost-url
    GIT_REPOSITORY https://github.com/boostorg/url.git
    GIT_TAG 954eb5b7e6af8228fa3b8a644c6a2e51b29980e7
)

FetchContent_GetProperties(boost-url)
if(NOT boost-url_POPULATED)
    FetchContent_Populate(boost-url)
    add_subdirectory(${boost-url_SOURCE_DIR} ${boost-url_BINARY_DIR})
endif()

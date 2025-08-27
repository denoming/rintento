set(TARGET rintento-wit)

add_library(${TARGET} STATIC)
add_library(Rintento::Wit ALIAS ${TARGET})

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC Jarvisto::Core
           Boost::headers
           Rintento::Coro
           Rintento::Common
    PRIVATE PkgConfig::OpenSSL
            PkgConfig::OpenCrypto
            Boost::url
            Boost::json
)

target_sources(${TARGET}
    PRIVATE src/RemoteRecognition.cpp
            src/MessageRecognition.cpp
            src/SpeechRecognition.cpp
            src/IntentParser.cpp
            src/RecognitionFactory.cpp
            src/Types.cpp
            src/Utils.cpp
            src/Config.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

target_compile_definitions(${TARGET}
    PRIVATE BOOST_ASIO_NO_DEPRECATED=1
)

if(ENABLE_TESTS)
    add_subdirectory(test)
endif()
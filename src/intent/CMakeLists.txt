set(TARGET jarvis-intent)

add_library(${TARGET} STATIC)
add_library(Jarvis::Intent ALIAS ${TARGET})

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC Jarvis::Common
    PRIVATE fmt::fmt OpenSSL::SSL OpenSSL::Crypto Boost::json
)

target_sources(${TARGET}
    PRIVATE src/Types.cpp
            src/PendingRecognition.cpp
            src/WitIntentSession.cpp
            src/WitMessageRecognition.cpp
            src/WitSpeechRecognition.cpp
            src/WitPendingRecognition.cpp
            src/WitIntentParser.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

target_compile_definitions(${TARGET}
    PRIVATE BOOST_BEAST_USE_STD_STRING_VIEW
    PRIVATE BOOST_ASIO_NO_TS_EXECUTORS
)
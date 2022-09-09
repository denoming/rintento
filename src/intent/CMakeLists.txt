set(TARGET jarvis-intent)

add_library(${TARGET} STATIC)
add_library(Jarvis::Intent ALIAS ${TARGET})

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC Jarvis::Common
    PUBLIC Boost::headers Boost::json
    PRIVATE OpenSSL::SSL OpenSSL::Crypto Boost::url
)

target_sources(${TARGET}
    PRIVATE src/Types.cpp
            src/HttpUtils.cpp
            src/Recognition.cpp
            src/WitRecognition.cpp
            src/WitMessageRecognition.cpp
            src/WitSpeechRecognition.cpp
            src/WitIntentParser.cpp
            src/WitRecognitionFactory.cpp
            src/RecognitionServer.cpp
            src/RecognitionDispatcher.cpp
            src/IntentPerformer.cpp
            src/IntentSubsystem.cpp
            src/RecognitionHandler.cpp
            src/RecognitionMessageHandler.cpp
            src/RecognitionSpeechHandler.cpp
            src/RecognitionTerminalHandler.cpp
            src/SpeechDataBuffer.cpp
            src/Utils.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

target_compile_definitions(${TARGET}
    PRIVATE BOOST_BEAST_USE_STD_STRING_VIEW
    PRIVATE BOOST_ASIO_NO_TS_EXECUTORS
)

target_precompile_headers(${TARGET}
    PRIVATE include/intent/Http.hpp
)
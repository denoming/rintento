set(TARGET jarvis-executor-intent)

add_library(${TARGET} STATIC)
add_library(Jarvis::Executor::Intent ALIAS ${TARGET})

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(${TARGET}
    PUBLIC Jarvis::Common
           Jarvis::Speaker
           Jarvis::Weather
           Jarvis::Executor::Common
           PkgConfig::SigCpp
           Boost::headers
           Boost::json
    PRIVATE OpenSSL::SSL
            OpenSSL::Crypto
            Boost::url
)

target_sources(${TARGET}
    PRIVATE src/Types.cpp
            src/WitRecognition.cpp
            src/WitMessageRecognition.cpp
            src/WitSpeechRecognition.cpp
            src/WitIntentParser.cpp
            src/WitRecognitionFactory.cpp
            src/WitTypes.cpp
            src/WitHelpers.cpp
            src/RecognitionServer.cpp
            src/RecognitionDispatcher.cpp
            src/ActionPerformer.cpp
            src/IntentSubsystem.cpp
            src/RecognitionHandler.cpp
            src/RecognitionMessageHandler.cpp
            src/RecognitionSpeechHandler.cpp
            src/RecognitionTerminalHandler.cpp
            src/SpeechDataBuffer.cpp
            src/Utils.cpp
            src/Action.cpp
            src/ActionRegistry.cpp
            src/PositioningClient.cpp
            src/registry/GetRainyStatusAction.cpp
            src/registry/GetAirQualityAction.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

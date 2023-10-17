set(TARGET rintento-intent)

add_library(${TARGET} STATIC)
add_library(Rintento::Intent ALIAS ${TARGET})

target_include_directories(${TARGET}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/options>
)

target_link_libraries(${TARGET}
    PUBLIC Jarvisto::Jarvisto
           Boost::headers
           Rintento::Coro
           Rintento::Wit
    PRIVATE Rintento::Process
            OpenSSL::SSL
            OpenSSL::Crypto
            Boost::url
            Boost::json
)

target_sources(${TARGET}
    PRIVATE src/RecognitionServer.cpp
            src/RecognitionSession.cpp
            src/IntentSubsystem.cpp
            src/RecognitionHandler.cpp
            src/RecognitionMessageHandler.cpp
            src/RecognitionSpeechHandler.cpp
            src/RecognitionTerminalHandler.cpp
            src/SpeechDataBuffer.cpp
            src/Utils.cpp
            src/Automation.cpp
            src/AutomationPerformer.cpp
            src/AutomationRegistry.cpp
            src/DeferredJob.cpp
            src/ScriptAction.cpp
            src/SequentLaunchStrategy.cpp
            src/Config.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

target_compile_definitions(${TARGET}
    PRIVATE BOOST_ASIO_NO_DEPRECATED=1
)

if(ENABLE_TESTS)
    add_subdirectory(test)
endif()
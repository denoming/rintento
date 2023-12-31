set(Python_FIND_VIRTUALENV FIRST)

find_package(Python COMPONENTS Interpreter)

if (NOT Python_FOUND)
    message(FATAL_ERROR "Python interpreter is not found")
endif()

set(PyTestExec ${Python_EXECUTABLE} -m pytest -p no:cacheprovider)
set(PyTestMain ${PyTestExec} --exec-path=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rintento)

add_test(
    NAME TestMessageRecognition
    COMMAND ${PyTestMain}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/test_message_recognition.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

set_tests_properties(TestMessageRecognition PROPERTIES LABELS "Component")

add_test(
    NAME TestSpeechRecognition
    COMMAND ${PyTestMain}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/test_speech_recognition.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

set_tests_properties(TestSpeechRecognition PROPERTIES LABELS "Component")

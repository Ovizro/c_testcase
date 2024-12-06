cmake_minimum_required(VERSION 3.15)
project(test)

set(TESTS_DIR .)
set(TEST_COMMON_SOURCE "${TESTS_DIR}/c_testcase.cpp")

file(GLOB_RECURSE TEST_FILES "${TESTS_DIR}/test_*.cpp" "${TESTS_DIR}/test_*.c")

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE} ${TEST_COMMON_SOURCE} ${SRC_LIST})
    list(APPEND TEST_EXECUTABLES "${EXECUTABLE_OUTPUT_PATH}/${TEST_NAME}")
endforeach()

add_custom_target(test
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/unittest.py ${TEST_EXECUTABLES}
    DEPENDS ${TEST_EXECUTABLES}
)

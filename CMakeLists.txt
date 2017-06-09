cmake_minimum_required(VERSION 2.8)

project(pnet)

SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Werror -std=c++11" )

add_subdirectory(test/)

# testing
enable_testing()
add_test(test_packet ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_packet)
add_test(test_packet ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_logger)

# Installation
set(INSTALL_DIR /usr/local/include/pnet)
install(DIRECTORY DESTINATION ${INSTALL_DIR})
install(DIRECTORY include/ DESTINATION ${INSTALL_DIR})

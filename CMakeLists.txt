cmake_minimum_required(VERSION 3.8)
project("picomodel")

set(PICOMODEL_VERSION 0.8.20)
add_definitions(-DPICOMODEL_VERSION="${PICOMODEL_VERSION}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

add_subdirectory("src")
add_subdirectory("tests")

add_dependencies(run_tests picomodel)

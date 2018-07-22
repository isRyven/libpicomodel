cmake_minimum_required(VERSION 3.8)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -Wall -O -march=native")
endif()

include_directories(AFTER "${CMAKE_SOURCE_DIR}/libs/catch2")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/src")

file(GLOB headers *.h)
add_executable(run_tests 
	main.cpp 
	pm_tests.cpp 
	pm_md3_tests.cpp 
	pm_mdc_tests.cpp 
	pm_obj_tests.cpp
    pm_ase_tests.cpp
	pm_mdl_tests.cpp
	${headers}
)

target_link_libraries(run_tests picomodel)

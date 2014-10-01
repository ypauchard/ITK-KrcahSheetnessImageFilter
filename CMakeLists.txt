cmake_minimum_required(VERSION 2.8)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(Testbench)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Testbench_SOURCE_DIR}/bin)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

subdirs(src)
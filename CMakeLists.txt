cmake_minimum_required(VERSION 2.8)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." OFF) # Makes boolean 'test' available.

project(Testbench)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Testbench_SOURCE_DIR}/bin)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

subdirs(src)

################################
# gtest
################################
if (test)
add_subdirectory(lib/gtest-1.7.0)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

##############
# Unit Tests
##############
add_executable(runUnitTests src/test_Sheetness.cxx)
target_link_libraries(runUnitTests gtest gtest_main)

endif()
cmake_minimum_required(VERSION 2.8)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0053 NEW)
endif(COMMAND cmake_policy)

# Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." OFF) # Makes boolean 'test' available.

# ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Include this project
include_directories(include)

option(BUILD_EXAMPLES "Build examples." ON)
if(BUILD_EXAMPLES)
	subdirs(example)
endif(BUILD_EXAMPLES)

################################
# gtest
################################
if (test)
	add_subdirectory(lib/googletest)
	enable_testing()
	set(GTEST_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
	include_directories(${GTEST_INCLUDE_DIRS})

	##############
	# Unit Tests
	##############
	subdirs(test)
endif()
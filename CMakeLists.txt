project(multipass_project)
cmake_minimum_required(VERSION 2.8)

# Set up dashboard and testing
include(CTest)
enable_testing()
include(${CMAKE_CURRENT_SOURCE_DIR}/CTestConfig.cmake)

# Comment out to disable the debug build
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_DEBUG_POSTFIX "d")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

# Verbosity is good, especially if a command fails
set(CMAKE_VERBOSE_MAKEFILE ON)
set(TARGET_ARCHITECTURE "x64")

# Ensure we use C++ 11 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
# Ensure we generate position independent code 
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()

# CMake modules are in cmake folder
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
message(STATUS "CMake Module Path: " ${CMAKE_MODULE_PATH})

add_subdirectory(src/)



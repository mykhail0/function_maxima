project(JNP)
cmake_minimum_required(VERSION 2.8.11)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

set(files
   ../src/function_maxima.h
)

# Locate GTest
find_package(GTest REQUIRED)

# Link runTests with what we want to test and the GTest library
add_executable(runTests maximaTest.cpp ${files})

target_link_libraries(runTests GTest::Main)
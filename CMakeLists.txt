cmake_minimum_required(VERSION 3.0)
project(JNP1_2)

# Set default compile flag for GCC
#if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Stops on first error:
    #set(CMAKE_CXX_FLAGS "-Wfatal-errors -std=c++17 -O0")
    set(CMAKE_CXX_FLAGS "-Wfatal-errors -Wall -Wextra -pedantic -std=c++17 -Og -g -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=address -fsanitize=undefined -fstack-protector")
    #set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17 -O0")
    #set(CMAKE_C_FLAGS "-Wfatal-errors -std=c11 -O0")
    #set(CMAKE_C_FLAGS "-Wall -Wextra -std=c11 -O0")
#endif ()

#add_executable(test src/test.cc)
add_definitions(-DTEST_NUM=101)
add_executable(testyDamiana src/test_damiana.cc)
add_executable(wyjatkowyInt src/wyjatkowy_int.cpp)
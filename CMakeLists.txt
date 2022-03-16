cmake_minimum_required(VERSION 3.10)

project(SimpleVector)
set(CMAKE_CXX_STANDARD 17)

add_executable("simple_vector" "src/main.cpp" "inc/array_ptr.h" "inc/simple_vector.h") 

cmake_minimum_required(VERSION 3.20)
project(RenderGraphCompiler)

set(CMAKE_CXX_STANDARD 20)
include_directories(include)
add_subdirectory(lib)
add_subdirectory(test)

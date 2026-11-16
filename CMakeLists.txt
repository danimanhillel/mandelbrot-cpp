cmake_minimum_required(VERSION 3.10)
project(MandelbrotExplorer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(mandelbrot main.cpp)
target_link_libraries(mandelbrot ${SDL2_LIBRARIES})

if(MSVC)
    target_compile_options(mandelbrot PRIVATE /O2 /fp:fast)
else()
    target_compile_options(mandelbrot PRIVATE -O3 -march=native -pthread)
    target_link_libraries(mandelbrot pthread)
endif()

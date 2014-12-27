cmake_minimum_required(VERSION 2.8.4)
project(advancedMouse)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)

add_executable(advancedMouse ${SOURCE_FILES})

target_link_libraries (advancedMouse evdev)
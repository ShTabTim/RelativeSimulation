cmake_minimum_required(VERSION 3.8)

project(sh_relative)

set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(${PROJECT_NAME} "main.cpp")
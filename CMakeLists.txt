cmake_minimum_required(VERSION 3.20)
project(srun LANGUAGES CXX C)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_library(glad STATIC glad/src/gl.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/glad/include)
file(GLOB BUTTERFLIES_SOURCES_C ${CMAKE_CURRENT_SOURCE_DIR} *.c glad/src/gl.c)

add_executable(srun src/main.c)

target_link_libraries(srun PUBLIC glad glfw OpenGL::GL OpenGL::GLU fmt)

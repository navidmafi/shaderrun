cmake_minimum_required(VERSION 3.20)
project(srun LANGUAGES C CXX)

find_package(OpenGL REQUIRED)
add_subdirectory(extern/glfw)


add_library(glad STATIC extern/glad/src/gl.c)

target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/include
)


add_executable(srun src/main.c)

target_link_libraries(srun PRIVATE
    glad
    glfw
    OpenGL::GL
    OpenGL::GLU
)

target_link_options(srun PRIVATE
    -static-libgcc
    -static-libstdc++
)

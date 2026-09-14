cmake_minimum_required(VERSION 3.10)
project(ShaderToy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(include)

add_executable(ShaderToy
    src/main.cpp
    src/glad.c
    src/Window.cpp
    src/ShaderProgram.cpp
    src/FileWatcher.cpp
    src/Quad.cpp
    src/TextureGenerator.cpp
)

target_link_libraries(ShaderToy OpenGL::GL glfw)

add_custom_command(TARGET ShaderToy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        ${CMAKE_CURRENT_BINARY_DIR}/shaders
)

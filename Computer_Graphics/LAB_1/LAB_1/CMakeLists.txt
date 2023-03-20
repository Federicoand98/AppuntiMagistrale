cmake_minimum_required(VERSION 3.24)
project(LAB_1)

set(CMAKE_CXX_STANDARD 17)
if (APPLE)
    set(CMAKE_CXX_FLAGS "-framework Cocoa -framework IOKit")
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif ()

add_executable(LAB_1 main.cpp ShaderMaker.cpp ShaderMaker.h)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(LAB_1 glfw glm::glm OpenGL::GL GLEW::GLEW)
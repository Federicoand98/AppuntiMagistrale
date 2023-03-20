cmake_minimum_required(VERSION 3.24)
project(2D_SIERPINSKY)

set(CMAKE_CXX_STANDARD 17)
if (APPLE)
    set(CMAKE_CXX_FLAGS "-framework Cocoa -framework IOKit")
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif ()

add_executable(2D_SIERPINSKY main.cpp ShaderMaker.h ShaderMaker.cpp)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(2D_SIERPINSKY glfw glm::glm OpenGL::GL GLEW::GLEW)
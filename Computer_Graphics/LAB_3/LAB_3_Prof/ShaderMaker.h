#pragma once

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>

// Creates a GLSL program object from vertex and fragment shader files
GLuint createProgram(std::string vShaderFile, std::string fShaderFile);

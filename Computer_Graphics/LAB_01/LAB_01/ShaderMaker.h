#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
using namespace std;

class ShaderMaker
{
    public:
		static GLuint createProgram(char* vertexfilename, char *fragmentfilename);
		static char* readShaderSource(const char* shaderFile);
	
    private:
		ShaderMaker() { }	
};
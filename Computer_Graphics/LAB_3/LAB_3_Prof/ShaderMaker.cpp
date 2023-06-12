#define _CRT_SECURE_NO_WARNINGS // for fscanf
#include "ShaderMaker.h"

//  Helper function to load vertex and fragment shader files
// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile)
{
	FILE* fp = fopen(shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);

	buf[size] = '\0';
	fclose(fp);

	return buf;
}


// Create a GLSL program object from vertex and fragment shader files
GLuint createProgram(std::string vShaderFile, std::string fShaderFile)
{
	struct Shader {
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	}  shaders[2] = {
		{ vShaderFile.data(), GL_VERTEX_SHADER, NULL },
		{ fShaderFile.data(), GL_FRAGMENT_SHADER, NULL }
	};

	//GLuint program = glCreateProgram();
	GLuint shader[2];
	GLuint program = glCreateProgram();

	for (int i = 0; i < 2; ++i) {
		Shader& s = shaders[i];
		s.source = readShaderSource(s.filename);
		if (shaders[i].source == NULL) {
			std::cerr << "\nFailed to read " << s.filename << std::endl;
			std::getchar();
			exit(EXIT_FAILURE);
		}

		shader[i] = glCreateShader(s.type);

		glAttachShader(program, shader[i]);
		glShaderSource(shader[i], 1, (const GLchar**)&s.source, NULL);
		glCompileShader(shader[i]);

		GLint  compiled;
		glGetShaderiv(shader[i], GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			std::cerr << s.filename << "\n failed to compile:" << std::endl;
			GLint  logSize;
			glGetShaderiv(shader[i], GL_INFO_LOG_LENGTH, &logSize);
			char* logMsg = new char[logSize];
			glGetShaderInfoLog(shader[i], logSize, NULL, logMsg);
			std::cerr << logMsg << std::endl;
			delete[] logMsg;
			std::getchar();
			exit(EXIT_FAILURE);
		}

		delete[] s.source;
	}

	/* link  and error check */
	glLinkProgram(program);

	GLint  linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		std::cerr << "\nShader program failed to link" << std::endl;
		GLint  logSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetProgramInfoLog(program, logSize, NULL, logMsg);
		std::cerr << logMsg << std::endl;
		delete[] logMsg;
		std::getchar();
		exit(EXIT_FAILURE);
	}

	return program;
}


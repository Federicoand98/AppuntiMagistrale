// 2D_TRIANGLE.cpp : 
//
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

static unsigned int programId;

unsigned int VAO;
unsigned int VBO;


float vertices[] = {
	//posizioni              //Colore
	-0.5f, -0.5f, 0.0f,  1.0f,0.0f,0.0f,1.0f, //vertice in basso a sinistra
	 0.5f, -0.5f, 0.0f,  0.0f,1.0f,0.0f,1.0f, //vertice in basso a destra
	 0.0f,  0.5f, 0.0f,  0.0f,0.0f,1.0f,1.0f  //vertice in alto
};

void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}

void init(void)
{

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a buffer for OpenGL to use
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Configura l'attributo posizione
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Configura l'attributo colore
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//set background color
	glClearColor(1.0, 0.5, 0.0, 1.0);   
}

void drawScene(void)
{

	glClear(GL_COLOR_BUFFER_BIT);
	
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glutSwapBuffers();
}
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:	// Escape key
		exit(1);
		break;
	case ' ': // Space bar
		glClearColor(1.0, 0.0, 0.0, 1.0);
		break;
	}

	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(800, 800);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Colored Triangle");
	
	// Callback functions
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(keyboard);
	
	glewExperimental = GL_TRUE;
	glewInit();
	
	initShader();
	init();

	glutMainLoop();
}

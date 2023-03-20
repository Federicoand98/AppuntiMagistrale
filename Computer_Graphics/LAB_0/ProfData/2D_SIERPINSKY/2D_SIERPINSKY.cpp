// Sierpinsky.cpp 

#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

static unsigned int programId;

unsigned int VAO, VAO_2;
unsigned int VBO, VBO_2;

using namespace glm;

typedef struct { float x, y, z; } Point;

int nPoint = 100000;
Point* Punti = new Point[nPoint];

int nPointPOL = 5;
Point* PuntiPOL = new Point[nPointPOL];

void sierpinski(void)
{
	Point T[3] = { {-0.5f, -0.5f, 0.0f} , {0.5f, -0.5f, 0.0f} , {0.0f, 0.5f, 0.0f} };
	
	int i, index = rand() % 3;
	// Scegliamo il punto iniziale  , a caso tra uno dei tre  vertici
	Punti[0] = T[index];

	for (i = 1; i < nPoint; i++)
	{
		//Scelgo a caso uno dei tre vertici iniziali
		index = rand() % 3;
		//Il nuovo punto è il punto medio tra il punto precedente
	  //ed il vertice scelto a caso.
		Punti[i].x = (Punti[i - 1].x + T[index].x) / 2;
		Punti[i].y = (Punti[i - 1].y + T[index].y) / 2;
		Punti[i].z = 0;
	}

	// square frame points
	PuntiPOL[0].x = -0.8;
	PuntiPOL[0].y = -0.8;
	PuntiPOL[0].z = 0;
	PuntiPOL[1].x = 0.8;
	PuntiPOL[1].y = -0.8;
	PuntiPOL[1].z = 0; 
	PuntiPOL[2].x = 0.8;
	PuntiPOL[2].y = 0.8;
	PuntiPOL[2].z = 0; 
	PuntiPOL[3].x = -0.8;
	PuntiPOL[3].y = 0.8;
	PuntiPOL[3].z = 0;
	PuntiPOL[4].x = -0.8;
	PuntiPOL[4].y = -0.8;
	PuntiPOL[4].z = 0;

}

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
	sierpinski();

	//Genero un VAO
	glGenVertexArrays(1, &VAO);
	//Ne faccio il bind (lo collego, lo attivo)
	glBindVertexArray(VAO);
	//AL suo interno genero un VBO
	glGenBuffers(1, &VBO);
	//Ne faccio il bind (lo collego, lo attivo, assegnandogli il tipo GL_ARRAY_BUFFER)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//Carico i dati vertices sulla GPU
	glBufferData(GL_ARRAY_BUFFER, nPoint * sizeof(Point), &Punti[0], GL_STATIC_DRAW);
	// Configurazione dell'attributo posizione: informo il vertex shader su: dove trova le informazioni sulle posizioni e come le deve leggre
	//dal buffer caricato sulla GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Square frame store
	glGenVertexArrays(1, &VAO_2);
	glBindVertexArray(VAO_2);
	glGenBuffers(1, &VBO_2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
	glBufferData(GL_ARRAY_BUFFER, nPointPOL * sizeof(Point), &PuntiPOL[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// 

	// Background color
	glClearColor(1.0, 0.5, 0.0, 1.0);
	glViewport(0, 0, 800, 800);

}
void drawScene(void)
{
	// Set every pixel in the frame buffer to the current clear color.
	glClear(GL_COLOR_BUFFER_BIT); 
	
	
	glBindVertexArray(VAO);
	glPointSize(4.0);
	glDrawArrays(GL_POINTS, 0, nPoint);

	glBindVertexArray(VAO_2);
	glLineWidth(2.5); 
	glDrawArrays(GL_LINE_STRIP, 0, nPointPOL);

	glutSwapBuffers();

}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(800, 800);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Sierpinsky 2D");
	glutDisplayFunc(drawScene);


	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glutMainLoop();
}


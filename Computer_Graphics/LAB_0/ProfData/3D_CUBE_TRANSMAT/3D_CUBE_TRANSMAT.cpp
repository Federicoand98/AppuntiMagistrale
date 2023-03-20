// 3D_CUBE_TRANSMAT.cpp 
// (advanced) Cube data defined with colored faces 
//
// 
// VIEWING:  (lookAt())
//		'a' /'A'		move camera position right and left 
//		's'/'S'			move camera position close /away 
// 
// PROJECTION (perspective() )		mouse wheel	change fov 
//
// MODELING:   translate()/rotate()/scale()  to transform the cube geometry 
//				special keys		spinning 

#include <iostream>

#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

// Include GLM; libreria matematica per le opengl
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

int		width = 800;
int		height = 800;
int		fov = 45;
float	zNear = 0.01;
float	zFar = 10000.0;

static unsigned int programId, MatProj, MatModel, MatView;

vec3 cameraPos = vec3(0.0, 0.0, 10.0);
vec3 cameraFront = vec3(0.0, 0.0, -1.0);
vec3 cameraUp = vec3(0.0, 1.0, 0.0);

float deltaTime = 0.0;  //tempo fra il frame corrente e l'ultimo
float last_frame = 0.0;  // tempo dell'ultimo frame

float rotateX = 15, rotateY = -15, rotateZ = 0;  // rotation amounts about axes, controlled by keyboard

//////////////////////////////////////////////////////////////////////////
// Cube data
unsigned int vao;
const int NumVertices = 36;
typedef  vec4  point4;
typedef  vec4  color4;
point4  vPositions[NumVertices];
color4  vColors[NumVertices];

point4 positions[8] = {
	point4(-0.5, -0.5,  0.5, 1.0),
	point4(-0.5,  0.5,  0.5, 1.0),
	point4(0.5,  0.5,  0.5, 1.0),
	point4(0.5, -0.5,  0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5,  0.5, -0.5, 1.0),
	point4(0.5,  0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

color4 colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 1.0, 1.0),  // white
	color4(0.0, 1.0, 1.0, 1.0)   // cyan
};
int Index = 0;  // global variable indexing into VBO arrays
void polygon(int a, int b, int c, int d)
{
	vColors[Index] = colors[a]; vPositions[Index] = positions[a]; Index++;
	vColors[Index] = colors[b]; vPositions[Index] = positions[b]; Index++;
	vColors[Index] = colors[c]; vPositions[Index] = positions[c]; Index++;
	vColors[Index] = colors[a]; vPositions[Index] = positions[a]; Index++;
	vColors[Index] = colors[c]; vPositions[Index] = positions[c]; Index++;
	vColors[Index] = colors[d]; vPositions[Index] = positions[d]; Index++;
}
void colorcube()
{
	polygon(1, 0, 3, 2);
	polygon(2, 3, 7, 6);
	polygon(3, 0, 4, 7);
	polygon(6, 5, 1, 2);
	polygon(4, 5, 6, 7);
	polygon(5, 4, 0, 1);
}
///////////////////////////////////////////////////////////////////////////////////

mat4 Projection, Model, View;

vec3 cubePositions[] = {
 vec3(0.0f,   0.0f,  0.0f),
 vec3(2.0f,   5.0f,-15.0f),
 vec3(-1.5f, -2.2f, -2.5f),
 vec3(-3.8f, -2.0f,-12.3f),
 vec3(2.4f,  -0.4f, -3.5f),
 vec3(-1.7f,  3.0f, -7.5f),
 vec3(1.3f,  -2.0f, -2.5f),
 vec3(1.5f,   2.0f, -2.5f),
 vec3(1.5f,   0.2f, -1.5f),
 vec3(-1.3f,  1.0f, -1.5f)
};

void initShader(void)
{
		GLenum ErrorCheckValue = glGetError();

		char* vertexShader = (char*)"vertexShader_C.glsl";
		char* fragmentShader = (char*)"fragmentShader_C.glsl";

		programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
		glUseProgram(programId);
}

void init(void)
{
	colorcube();

	// Create a vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// 1st attribute VBO : positions 
	unsigned int vpositionID;
	glGenBuffers(1, &vpositionID);
	glBindBuffer(GL_ARRAY_BUFFER, vpositionID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vPositions), vPositions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vpositionID);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attribute VBO : colors 
	unsigned int vcolorID;
	glGenBuffers(1, &vcolorID);
	glBindBuffer(GL_ARRAY_BUFFER, vcolorID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vColors), vColors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vcolorID);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	
	//Definisco il colore che verrà assegnato allo schermo
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// Ottieni l'identificativo della variabile uniform mat4 Projection (in vertex shader).
    //Questo identificativo sarà poi utilizzato per il trasferimento della matrice Projection al Vertex Shader
	MatProj = glGetUniformLocation(programId, "Projection");
	// Ottieni l'identificativo della variabile uniform mat4 Model (in vertex shader)
	//Questo identificativo sarà poi utilizzato per il trasferimento della matrice Model al Vertex Shader
	MatModel = glGetUniformLocation(programId, "Model");
	//Ottieni l'identificativo della variabile uniform mat4 View (in vertex shader)
	//Questo identificativo sarà poi utilizzato per il trasferimento della matrice View al Vertex Shader
	MatView = glGetUniformLocation(programId, "View");

}

void mousewheel(int button, int dir, int x, int y)
{
	if (dir > 0)
		fov += 1;
	else
		fov -= 1;
	glutPostRedisplay();
}
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
}

void mykeyboard(unsigned char key, int x, int y)
{
	float alfa = 0.05 * deltaTime;

	switch (key)

	{
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case 'a': //move camera to the right
		vec3 dir = normalize(cross(cameraFront, cameraUp));
		cameraPos = cameraPos - alfa * dir;
		break;

	case 'A': //move camera to the left 
		vec3 direzione = normalize(cross(cameraFront, cameraUp));
		cameraPos = cameraPos + alfa * direzione;
		break;

	case 's': // move camera away (same direction)
		cameraPos = cameraPos - alfa * cameraFront;
		break;

	case 'S': // move camera close (same direction)
		cameraPos = cameraPos + alfa * cameraFront;
		break;
	case ' ':  // reset values to their defaults
		cameraPos = vec3(0.0, 0.0, 10.0);
		cameraFront = vec3(0.0, 0.0, -1.0);
		cameraUp = vec3(0.0, 1.0, 0.0);

		break;
	}

	glutPostRedisplay();
}

void specialKeyFunction(int key, int x, int y) {
	// called when a special key is pressed 
	if (key == GLUT_KEY_LEFT)
		rotateY -= 15;
	else if (key == GLUT_KEY_RIGHT)
		rotateY += 15;
	else if (key == GLUT_KEY_DOWN)
		rotateX += 15;
	else if (key == GLUT_KEY_UP)
		rotateX -= 15;
	else if (key == GLUT_KEY_PAGE_UP)
		rotateZ += 15;
	else if (key == GLUT_KEY_PAGE_DOWN)
		rotateZ -= 15;
	else if (key == GLUT_KEY_HOME)
		rotateX = rotateY = rotateZ = 0;
	glutPostRedisplay();
}


void drawScene(void)
{
	int i, n_cubi = 5;  // fino a 10 cubi
	float timevalue = glutGet(GLUT_ELAPSED_TIME) * 0.0001;

	//Passo al Vertex Shader il puntatore alla matrice Projection, che sarà associata alla variabile Uniform mat4 Projection
	//all'interno del Vertex shader. Uso l'identificatio MatProj
	Projection = perspective(radians((float)fov), (float)(width) / float(height), zNear, zFar);
	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	//Costruisco la matrice di Vista che applicata ai vertici in coordinate mondo WCS
	//li trasforma nel sistema di riferimento della camera VCS.
	// usage: lookAt(eye,at,up);
	View = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glUniformMatrix4fv(MatView, 1, GL_FALSE, value_ptr(View));

	float currentFrame = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = currentFrame - last_frame;
	last_frame = currentFrame;
	glClearColor(1.0, 0.0, 1.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (i = 0; i < n_cubi; i++)
	{
		Model = mat4(1.0);
		Model = translate(Model, cubePositions[i]);
		Model = rotate(Model, radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f)); 
		Model = rotate(Model, radians(rotateY), glm::vec3(0.0f, 1.0f, 0.0f));
		Model = rotate(Model, radians(rotateZ), glm::vec3(0.0f, 0.0f, 1.0f));
		Model = scale(Model, vec3(2.0f, 2.0f, 2.0f));

		//Passo al Vertex Shader il puntatore alla matrice Model, che sarà associata alla variabile Uniform mat4 Projection
		//all'interno del Vertex shader. Uso l'identificatio MatModel
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	
		glBindVertexArray(vao);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}
	glutSwapBuffers();
}

void update(int a)
{
	glutTimerFunc(20, update, 0);
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("(Advanced) Cubes 3D");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	
	glutMouseWheelFunc(mousewheel);
	glutKeyboardFunc(mykeyboard);
	glutSpecialFunc(specialKeyFunction);

	glutTimerFunc(20, update, 0);
	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glEnable(GL_ALPHA_TEST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glutMainLoop();
}
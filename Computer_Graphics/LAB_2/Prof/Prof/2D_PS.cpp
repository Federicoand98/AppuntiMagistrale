// ParticleSystem.cpp
//

//#include <cmath>
#include <time.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <vector>
#include "ShaderMaker.h"

static unsigned int programId;

unsigned int VAO;
unsigned int VBO;

using namespace std;

typedef struct {
	float r;
	float g;
	float b;
} color;

typedef struct {
	float x;
	float y;
	float alpha;
	float xFactor;
	float yFactor;
	float drag;
	color color;
} PARTICLE;

vector <PARTICLE> particles;

typedef struct { float x, y, z, r, g, b, a; } Point;
int nPoint = 5000;
Point* Punti = new Point[nPoint];

//Viewport size
float width = 800; //1366;
float height = 768.0;

void setup() {
	float r = 0.0, g = 0.0, b = 0.0;
	glClearColor(r, g, b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	setup();
}

color computeRainbow() {
	static float rgb[3] = { 1.0, 0.0, 0.0 };
	static int fase = 0, counter = 0;
	const float step = 0.1;
	color paint;

	switch (fase) {
	case 0: rgb[1] += step;
		break;
	case 1: rgb[0] -= step;
		break;
	case 2: rgb[2] += step;
		break;
	case 3: rgb[1] -= step;
		break;
	case 4: rgb[0] += step;
		break;
	case 5: rgb[2] -= step;
		break;
	default:
		break;
	}
	//fprintf(stdout, "Rosso e verde e blu: %f,%f,%f, counter= %i\n", rgb[0], rgb[1], rgb[2], counter);

	counter++;
	if (counter > 1.0 / step) {
		counter = 0;
		fase < 5 ? fase++ : fase = 0;
	}

	paint.r = rgb[0];
	paint.g = rgb[1];
	paint.b = rgb[2];
	return paint;
}

void mouseMotion(int x, int	y) {
	color rgb = computeRainbow();
	for (int i = 0; i < 10; i++) {
		PARTICLE p;
		p.x = (float)x;
		p.y = (float)(height - y);
		p.alpha = 1.0;
		p.drag = 1.05;
		p.xFactor = (rand() % 1000 + 1) / 300.0 * (rand() % 2 == 0 ? -1 : 1);
		p.yFactor = (rand() % 1000 + 1) / 300.0 * (rand() % 2 == 0 ? -1 : 1);
		p.color.r = rgb.r;
		p.color.g = rgb.g;
		p.color.b = rgb.b;
		// Adds the new element p at the end of the vector, after its current last element
		particles.push_back(p); 
	}
}

void drawScene() {

	int P_size = 0; // particles.size();
	// For each particle that is(still) alive we update the values :
	for (int i = 0; i < particles.size(); i++) {
		particles.at(i).xFactor /= particles.at(i).drag;
		particles.at(i).yFactor /= particles.at(i).drag;

		particles.at(i).x += particles.at(i).xFactor;
		particles.at(i).y += particles.at(i).yFactor;

		particles.at(i).alpha -= 0.05;  // reduce life

		float xPos = -1.0f + ((float)particles.at(i).x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(particles.at(i).y)) * 2 / ((float)(height));

		if (particles.at(i).alpha <= 0.0) { // particle is dead
			particles.erase(particles.begin() + i);
		}
		else { // particle is alive, thus update
			Punti[i].x = xPos;
			Punti[i].y = yPos;
			Punti[i].z = 0.0;
			Punti[i].r = particles.at(i).color.r;
			Punti[i].g = particles.at(i).color.g;
			Punti[i].b = particles.at(i).color.b;
			Punti[i].a = particles.at(i).alpha;
			P_size += 1;
		}
	}
	setup();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, P_size * sizeof(Point), &Punti[0], GL_STATIC_DRAW);
	// Configura l'attributo posizione
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Configura l'attributo colore
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glPointSize(3.0);
	glDrawArrays(GL_POINTS, 0, P_size);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glutSwapBuffers();
}

void update(int value) {
	glutPostRedisplay();
	glutTimerFunc(10, update, 0);
}

void log(int value) {
	fprintf(stdout, "Particelle in memoria: %i\n", particles.size());
	glutTimerFunc(1000, log, 0);
}

void init(void)
{
	//PS
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//set background color
	glClearColor(0.0, 0.0, 0.0, 1.0);
}
void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Sistema Particellare");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutTimerFunc(20, update, 0);
	glutTimerFunc(500, log, 0);
	glutMotionFunc(mouseMotion);
	glutPassiveMotionFunc(mouseMotion);

	setup();
	srand((unsigned)time(NULL));

	glewExperimental = GL_TRUE;
	glewInit();
	fprintf(stdout, "INFO: OpenGL Version: %s\n", glGetString(GL_VERSION));

	initShader();
	init();

	glutMainLoop();

}
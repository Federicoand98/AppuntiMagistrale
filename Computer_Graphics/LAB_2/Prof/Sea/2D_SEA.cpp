// 2D_SEA.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//
// Con il tasto ‘r’/'R' il cannone ruota destra/sinistra
// Con il tasto ‘b’ viene visualizzato il bounding box della farfalla.
// con la barra spaziatrice il cannone spara un proiettile, 

// TASK: l’obiettivo è distruggere la farfalla. 
// Quando la farfalla viene colpita non viene più disegnata.

//NOTA La farfalla è disegnata con la curva parametrica che la descrive, 
// il cannone è fatto con le primitive di base.

#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
using namespace glm;
using namespace std;

static unsigned int programId, programId_text;
bool Coll = FALSE;
#define  PI   3.14159265358979323846
int frame_animazione = 0; // usato per animare la fluttuazione
unsigned int VAO_Text, VBO_Text;
float angolo = 0.0f;
float s = 1, dx = 0, dy = 0, dx_f = 0, dy_f = 0;
mat4 Projection;
GLuint MatProj, MatModel, loctime, locres;
float posx_Proiettile, posy_Proiettile, angoloCannone = 0.0;
int nv_P;

// viewport size
int width = 1000;
int height = 720;
float w_update, h_update;

bool drawBB = FALSE;
int posx = (float)width / 2.0, posy = (float)height / 4.0;

//STRUTTURA FIGURA
typedef struct {
	GLuint VAO;
	GLuint VBO_G;
	GLuint VBO_C;
	int nTriangles;
	// Vertici
	vector<vec3> vertici;
	vector<vec4> colors;
	// Numero vertici
	int nv;
	//Matrice di Modellazione: Traslazione*Rotazione*Scala
	mat4 Model;
	vec4 corner_b_obj;
	vec4 corner_t_obj;
	vec4 corner_b;
	vec4 corner_t;
	bool alive;
} Figura;

//SCENA
vector<Figura> Scena;

Figura Farf = {};
Figura Cannone = {};
Figura Proiettile = {};
Figura Cielo = {};
Figura Mare = {};


void updateProiettile(int value)
{
	//Ascissa del proiettile durante lo sparo
	posx_Proiettile = 0;
	//Ordinata del proettile durante lo sparo
	posy_Proiettile += 4;

	//L'animazione deve avvenire finchè  l'ordinata del proiettile raggiunge un certo valore fissato
	if (posy_Proiettile <= 800)
		glutTimerFunc(5, updateProiettile, 0);
	else
		posy_Proiettile = 0;
	//printf("posx posy %f %f \n", posx_Proiettile, posy_Proiettile);
	glutPostRedisplay();
}

void crea_VAO_Vector(Figura* fig)
{
	glGenVertexArrays(1, &fig->VAO);
	glBindVertexArray(fig->VAO);
	//Genero , rendo attivo, riempio il VBO della geometria dei vertici
	glGenBuffers(1, &fig->VBO_G);
	glBindBuffer(GL_ARRAY_BUFFER, fig->VBO_G);
	glBufferData(GL_ARRAY_BUFFER, fig->vertici.size() * sizeof(vec3), fig->vertici.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	//Genero , rendo attivo, riempio il VBO dei colori
	glGenBuffers(1, &fig->VBO_C);
	glBindBuffer(GL_ARRAY_BUFFER, fig->VBO_C);
	glBufferData(GL_ARRAY_BUFFER, fig->colors.size() * sizeof(vec4), fig->colors.data(), GL_STATIC_DRAW);
	//Adesso carico il VBO dei colori nel layer 2
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

}
void costruisci_cielo(vec4 color_top, vec4 color_bot, Figura* cielo)
{
	cielo->vertici.push_back(vec3(0.0, 0.0, 0.0));
	cielo->colors.push_back(vec4(color_bot.r, color_bot.g, color_bot.b, color_top.a));
	cielo->vertici.push_back(vec3(1.0, 0.0, 0.0));
	cielo->colors.push_back(vec4(color_bot.r, color_bot.g, color_bot.b, color_bot.a));
	cielo->vertici.push_back(vec3(0.0, 1.0, 0.0));
	cielo->colors.push_back(vec4(color_top.r, color_top.g, color_top.b, color_top.a));
	cielo->vertici.push_back(vec3(1.0, 1.0, 0.0));
	cielo->colors.push_back(vec4(color_top.r, color_top.g, color_top.b, color_top.a));

	cielo->nv = cielo->vertici.size();

	//Costruzione matrice di Modellazione Sole, che rimane la stessa(non si aggiorna)
	cielo->Model = mat4(1.0);
	cielo->Model = translate(cielo->Model, vec3(0.0, float(height) / 2, 0.0));
	cielo->Model = scale(cielo->Model, vec3(float(width), float(height) / 2, 1.0));
}

void costruisci_mare(vec4 color_top, vec4 color_bot, Figura* mare)
{
	mare->vertici.push_back(vec3(0.0, 0.0, 0.0));
	mare->colors.push_back(vec4(color_top.r, color_top.g, color_top.b, color_top.a));
	mare->vertici.push_back(vec3(0.0, 1.0, 0.0));
	mare->colors.push_back(vec4(color_top.r, color_top.g, color_top.b, color_top.a));
	mare->vertici.push_back(vec3(1.0, 0.0, 0.0));
	mare->colors.push_back(vec4(color_top.r, color_top.g, color_top.b, color_top.a));
	mare->vertici.push_back(vec3(1.0, 1.0, 0.0));
	mare->colors.push_back(vec4(color_bot.r, color_bot.g, color_bot.b, color_bot.a));

	mare->nv = mare->vertici.size();

	//Costruzione matrice di Modellazione mare, che rimane la stessa(non si aggiorna)
	mare->Model = mat4(1.0);
	mare->Model = scale(mare->Model, vec3(float(width), float(height) / 2, 1.0));
}


void costruisci_cannone(float cx, float cy, float raggiox, float raggioy, Figura* fig) {
	int i;
	float stepA = (PI) / fig->nTriangles;
	float t;


	fig->vertici.push_back(vec3(cx, cy, 0.0));
	fig->colors.push_back(vec4(255.0 / 255.0, 75.0 / 255.0, 0.0, 1.0));

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA + PI;
		fig->vertici.push_back(vec3(cx + raggiox * cos(t), cy + raggioy * sin(t), 0.0));
		//Colore 
		fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	}

	//Primo trapezio
	fig->vertici.push_back(vec3(-1.0, 0.0, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(1.0, 0.0, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(0.5, 2.5, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(0.5, 2.5, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(-1.0, 0.0, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(-0.5, 2.5, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));


	//Secondo trapezio al contrario
	fig->vertici.push_back(vec3(0.5, 2.5, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(1.0, 3.0, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(-0.5, 2.5, 0.0));

	fig->vertici.push_back(vec3(1.0, 3.0, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(-1.0, 3.0, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(-0.5, 2.5, 0.0));
	fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));

	fig->nv = fig->vertici.size();
}

void costruisci_proiettile(float cx, float cy, float raggiox, float raggioy, Figura* fig) {

	int i;
	float stepA = (2 * PI) / fig->nTriangles;
	float t;
	float xmax = 0;
	float xmin = 0;

	float ymax = 0;
	float ymin = 0;

	fig->vertici.push_back(vec3(cx, cy, 0.0));

	fig->colors.push_back(vec4(255.0 / 255.0, 75.0 / 255.0, 0.0, 1.0));

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA;
		fig->vertici.push_back(vec3(cx + raggiox * cos(t), cy + raggioy * sin(t), 0.0));
		//Colore 
		fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	}

	fig->nv = fig->vertici.size();

	//Costruisco xmin,ymin, xmax,ymax peer identificare il Bounding Box del Proiettile
	for (i = 1; i < fig->nv; i++)
	{

		if (fig->vertici[i].x < xmin)
			xmin = fig->vertici[i].x;
	}

	for (i = 1; i < fig->nv; i++)
	{

		if (fig->vertici[i].x > xmax)
			xmax = fig->vertici[i].x;
	}
	for (i = 1; i < fig->nv; i++)
	{
		if (fig->vertici[i].y <= ymin)
			ymin = fig->vertici[i].y;
	}

	for (i = 1; i < fig->nv; i++)
	{
		if (fig->vertici[i].y > ymax)
			ymax = fig->vertici[i].y;
	}
	//Aggiorno i valori del corner più in basso a sinistra (corner_b) e del corner più in alto a destra (conrner_t)

	fig->corner_b_obj = vec4(xmin, ymin, 0.0, 1.0);
	fig->corner_t_obj = vec4(xmax, ymax, 0.0, 1.0);
	//Aggiungo i vertici della spezzata per costruire il bounding box
	fig->vertici.push_back(vec3(xmin, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmin, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmin, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));

	//Aggiorno il numero dei vertici della figura
	fig->nv = fig->vertici.size();
}

void costruisci_farfalla(float cx, float cy, float raggiox, float raggioy, Figura* fig) {

	int i;
	float stepA = (2 * PI) / fig->nTriangles;
	float t;
	float xmax = 0;
	float xmin = 0;
	float ymax = 0;
	float ymin = 0;

	fig->vertici.push_back(vec3(cx, cy, 0.0));
	fig->colors.push_back(vec4(150.0 / 255.0, 75.0 / 255.0, 0.0, 1.0));

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA;
		fig->vertici.push_back(vec3(cx + raggiox * (sin(t) * (exp(cos(t)) - 2 * cos(4 * t)) + pow(sin(t / 12), 5)) / 4, cy + raggioy * (cos(t) * (exp(cos(t)) - 2 * cos(4 * t)) + pow(sin(t / 12), 5)) / 4, 0.0));
		//Colore 
		fig->colors.push_back(vec4(1.0, 0.0, 0.0, 0.0));
	}
	fig->nv = fig->vertici.size();

	//Calcolo di xmin, ymin, xmax, ymax
	for (i = 1; i < fig->nv; i++)
		if (fig->vertici[i].x <= xmin)
			xmin = fig->vertici[i].x;

	for (i = 1; i < fig->nv; i++)
		if (fig->vertici[i].x > xmax)
			xmax = fig->vertici[i].x;

	for (i = 1; i < fig->nv; i++)
		if (fig->vertici[i].y <= ymin)
			ymin = fig->vertici[i].y;

	for (i = 1; i < fig->nv; i++)
		if (fig->vertici[i].y > ymax)
			ymax = fig->vertici[i].y;

	//Aggiorno i valori del corner più in basso a sinistra (corner_b) e del corner più in alto a destra (conrner_t)
	fig->corner_b_obj = vec4(xmin, ymin, 0.0, 1.0);
	fig->corner_t_obj = vec4(xmax, ymax, 0.0, 1.0);
	//Aggiungo i vertici della spezzata per costruire il bounding box
	fig->vertici.push_back(vec3(xmin, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmin, ymin, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmin, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	fig->vertici.push_back(vec3(xmax, ymax, 0.0));
	fig->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));

	//Aggiorno il numero dei vertici della figura
	fig->nv = fig->vertici.size();
}

void init_shader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader_M.glsl";
	char* fragmentShader = (char*)"FS_mare.glsl";
	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);
}

void init(void)
{
	Farf.nTriangles = 180;
	costruisci_farfalla(0.0, 0.0, 1.0, 1.0, &Farf);
	crea_VAO_Vector(&Farf);
	Farf.alive = TRUE;
	Scena.push_back(Farf);

	Cannone.nTriangles = 180;
	costruisci_cannone(0.0, 0.0, 1.0, 1.0, &Cannone);
	crea_VAO_Vector(&Cannone);
	Scena.push_back(Cannone);

	Proiettile.nTriangles = 180;
	costruisci_proiettile(0.0, 0.0, 1.0, 1.0, &Proiettile);
	crea_VAO_Vector(&Proiettile);
	Scena.push_back(Proiettile);

	vec4 col_top = { 0.0, 0.0,1.0,1.0 };
	vec4 col_bottom = { 0.0, 0.0,0.0,1.0 };
	costruisci_cielo(col_top, col_bottom, &Cielo);
	crea_VAO_Vector(&Cielo);
	Scena.push_back(Cielo);

	col_top = vec4{ 0.1333, 0.1451, 0.9333, 1.0000 };
	col_bottom = vec4{ 0.01, 0.1,1.0, 1.0000 };
	costruisci_mare(col_top, col_bottom, &Mare);
	crea_VAO_Vector(&Mare);
	Scena.push_back(Mare);

	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");
	loctime = glGetUniformLocation(programId, "time");
	locres = glGetUniformLocation(programId, "resolution");
}

bool checkCollision(Figura obj1, Figura obj2) {
	// check collisioni su asse x
	bool collisionX = obj1.corner_b.x <= obj2.corner_t.x &&
		obj1.corner_t.x >= obj2.corner_b.x;
	// check collisioni su asse y
	bool collisionY = obj1.corner_b.y <= obj2.corner_t.y &&
		obj1.corner_t.y >= obj2.corner_b.y;
	//Ha collisione se c'è collisione sia nella direzione x che nella direzione y
	return collisionX && collisionY;
}
double  degtorad(double angle) {
	return angle * PI / 180;
}
void update_Cannone(int value)
{
	//Aggiorno l'angolo di fluttuazione del cannone
	frame_animazione += 1;
	if (frame_animazione >= 360) {
		frame_animazione -= 360;

	}
	angoloCannone = cos(degtorad(frame_animazione)) * 0.1;

	glutTimerFunc(25, update_Cannone, 0);
}
void drawScene(void)
{
	int i = 0, j = 0;
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(14.0);
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	vec2 resolution = vec2(w_update, h_update);

	glUniform1f(loctime, time);
	glUniform2f(locres, resolution.x, resolution.y);
	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));

	//Disegna Cielo e Mare
	for (i = Scena.size() - 2; i < Scena.size(); i++)
	{
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[i].Model));
		glBindVertexArray(Scena[i].VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, Scena[i].nv);
		glBindVertexArray(0);
	}
	/*Matrice di modellazione del cannone */
	Scena[1].Model = mat4(1.0);
	Scena[1].Model = translate(Scena[1].Model, vec3(600.0 + dx, 300.0 + dy, 0.0));
	Scena[1].Model = rotate(Scena[1].Model, angoloCannone, vec3(0.0f, 0.0f, 1.0f));
	Scena[1].Model = rotate(Scena[1].Model, angolo, vec3(0.0f, 0.0f, 1.0f));

	/*Matrice di modellazione del proiettile*/
	Scena[2].Model = translate(Scena[1].Model, vec3(posx_Proiettile, 80 + posy_Proiettile, 0.0));
	Scena[2].Model = scale(Scena[2].Model, vec3(30.5, 30.5, 1.0));
	Scena[2].corner_b = Scena[2].corner_b_obj;
	Scena[2].corner_t = Scena[2].corner_t_obj;
	Scena[2].corner_b = Scena[2].Model * Scena[2].corner_b;
	Scena[2].corner_t = Scena[2].Model * Scena[2].corner_t;
	 /*Update Scala Matrice di modellazione del cannone */
	Scena[1].Model = scale(Scena[1].Model, vec3(60.0, 60.5, 1.0));

	//Disegno Proiettile
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[2].Model));
	glBindVertexArray(Scena[2].VAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, Scena[2].nv - 6);

	if (drawBB == TRUE)
	{
		//Disegno Bounding Box
		glDrawArrays(GL_LINE_STRIP, Scena[2].nv - 6, 6);
		glBindVertexArray(0);
	}

	//Disegno Cannone
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[1].Model));
	glBindVertexArray(Scena[1].VAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, Scena[1].nv - 12);
	glDrawArrays(GL_TRIANGLES, Scena[1].nv - 12, 12);
	glBindVertexArray(0);

	/*Matrice di modellazione farfalla */
	Scena[0].Model = mat4(1.0);
	Scena[0].Model = translate(Scena[0].Model, vec3(900.0 - dx_f, 500.0 - dy_f, 0.0));
	Scena[0].Model = scale(Scena[0].Model, vec3(80.5, 80.5, 1.0));

	Scena[0].corner_b = Scena[0].corner_b_obj;
	Scena[0].corner_t = Scena[0].corner_t_obj;
	//printf("Farfalla \n");
	Scena[0].corner_b = Scena[0].Model * Scena[0].corner_b;
	//std::cout << glm::to_string(Scena[0].corner_b) << std::endl;
	Scena[0].corner_t = Scena[0].Model * Scena[0].corner_t;
	//std::cout << glm::to_string(Scena[0].corner_t) << std::endl;

	Coll = checkCollision(Scena[0], Scena[2]);

	if (Coll == TRUE)
		Scena[0].alive = FALSE;

	//Disegno la farfalla fino a quando non sia avvenuta la prima collisione con la palla del cannone
	if (Scena[0].alive == TRUE)
	{
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Scena[0].Model));
		glBindVertexArray(Scena[0].VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, Scena[0].nv - 6);
		if (drawBB == TRUE)
			//Disegno Bounding Box
			glDrawArrays(GL_LINE_STRIP, Scena[0].nv - 6, 6);
		glBindVertexArray(0);
	}
	glutSwapBuffers();
}

void keyboardReleasedEvent(unsigned char key, int x, int y)
{
	{
		switch (key)
		{
		case 'b':
			drawBB = FALSE;
			break;
		default:
			break;
		}
	}
	glutPostRedisplay();
}
void myKeyboard(unsigned char key, int x, int y)
{
	{
		switch (key)
		{
		case ' ':
			updateProiettile(0);
			break;

		case 'a':
			dx -= 1;
			break;

		case 'd':
			dx += 1;
			break;

		case 'R':
			angolo += 0.1;
			break;
		case 'r':
			angolo -= 0.1;
			break;

		case 't':
			angolo += 0.1;
			break;

		case 'b':
			drawBB = TRUE;
			break;
		default:
			break;
		}
	}
	glutPostRedisplay();
}
void update_f(int value)
{
	dx_f = dx_f + 0.1;
	dy_f = dy_f - 0.1;
	glutTimerFunc(25, update_f, 0);
	glutPostRedisplay();
}
void reshape(int w, int h)
{
	Projection = ortho(0.0f, (float)width, 0.0f, (float)height);

	float AspectRatio_mondo = (float)(width) / (float)(height); //Rapporto larghezza altezza di tutto ciò che è nel mondo
	 //Se l'aspect ratio del mondo è diversa da quella della finestra devo mappare in modo diverso 
	 //per evitare distorsioni del disegno
	if (AspectRatio_mondo > w / h)   //Se ridimensioniamo la larghezza della Viewport
	{
		glViewport(0, 0, w, w / AspectRatio_mondo);
		w_update = (float)w;
		h_update = w / AspectRatio_mondo;
	}
	else {  //Se ridimensioniamo la larghezza della viewport oppure se l'aspect ratio tra la finestra del mondo 
			//e la finestra sullo schermo sono uguali
		glViewport(0, 0, h * AspectRatio_mondo, h);
		w_update = h * AspectRatio_mondo;
		h_update = (float)h;
	}
}
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Shooting in the sea");

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(myKeyboard);
	glutReshapeFunc(reshape);
	glutKeyboardUpFunc(keyboardReleasedEvent);
	glutTimerFunc(25, update_f, 0);
	glutTimerFunc(25, update_Cannone, 0);

	glewExperimental = GL_TRUE;
	glewInit();
	init_shader();
	init();

	//Gestione della Trasparenza
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glutMainLoop();
}
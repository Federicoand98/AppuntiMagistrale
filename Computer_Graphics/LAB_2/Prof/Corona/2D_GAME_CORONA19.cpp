#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

static unsigned int programId;
#define PI 3.14159265358979323846

unsigned int VAO, VAO_SPACE, VAO_NEMICO;
unsigned int VBO, VBO_C, VBO_N, loc, MatProj, MatModel;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

vec4 col_bianco =	{ 1.0,1.0,1.0, 1.0 };
vec4 col_rosso =	{ 1.0,0.0,0.0, 1.0 };
vec4 col_nero =		{ 0.0,0.0,0.0, 1.0 };
vec4 col_magenta =	{ 1.0,1.0,0.0, 1.0 };

int NumeroColpiti = 0;
mat4 Projection;  //Matrice di proiezione
mat4 Model; //Matrice per il cambiamento da Sistema diriferimento dell'oggetto OCS a sistema Mondo WCS
typedef struct { float x, y, r, g, b, a; } Point;

float dxnemici = 0;
float dynemici = 0;
float posxN, posyN;
int nemici_per_riga = 5;
int numero_di_righe = 3;

int nTriangles = 15;
int nVertices_Navicella = 12 * nTriangles + 1;

int nvBocca = 5;
int nvTentacoli = 16;
int nVertices_Nemico = 3 * nTriangles + 2* 3 * nTriangles + nvBocca + nvTentacoli;

Point* Navicella = new Point[nVertices_Navicella];
Point* Nemico = new Point[nVertices_Nemico];

int vertices_space = 6;
Point* Space = new Point[vertices_space];

// Viewport size
int width = 1200;
int height = 720;

float posx_Proiettile = 0, posy_Proiettile = 0;

//ANIMARE V
double VelocitaOrizzontale = 0; //velocita orizzontale (pixel per frame)
int scuoti = 0;
float posx = width / 2; //coordinate sul piano della posizione iniziale della navicella
float posy = height * 0.2;
float posizione_di_equilibrio = posy;
float angolo = 0;

bool pressing_left = false;
bool pressing_right = false;
bool pressing_attack = false;

bool** colpito;
float angoloV = 0;
double range_fluttuazione = 15; // fluttuazione su-giù 
double angle = 0;				// angolo di fluttuazione
double angle_offset = 10; // quanto   accentuata è l'oscillazione angolare
double float_yoffset = 0; // distacco dalla posizione d'equilibrio 
int frame_animazione = 0; // usato per animare la fluttuazione
int frame = 0;
bool start = 0;

float lerp(float a, float b, float t) {
	//Interpolazione lineare tra a e b secondo amount
	return (1 - t) * a + t * b;
}

double  degtorad(double angle) {
	return angle * PI / 180;
}

void updateNemici(int value)
{
	frame++;
	if (frame % 30 == 0)
	{
		dxnemici--;
		dynemici--;
	}
	glutTimerFunc(5, updateNemici, 0);
	glutPostRedisplay();
}

void update(int a)
{
	float timeValue = glutGet(GLUT_ELAPSED_TIME);
	glutPostRedisplay();
	glutTimerFunc(50, update, 0);
}
void updateProiettile(int value)
{
	//Ascissa del proiettile durante lo sparo
	posx_Proiettile = 0;
	//Ordinata del proettile durante lo sparo
	posy_Proiettile++;

	//L'animazione deve avvenire finchè  l'ordinata del proiettile raggiunge un certo valore fissato
	if (posy_Proiettile <= 500)
		glutTimerFunc(5, updateProiettile, 0);
	else
		posy_Proiettile = 0;

	glutPostRedisplay();
}

// Aggioramento della posizione navicella
void updateV(int a)
{
	bool moving = false;

	if (pressing_left)
	{
		VelocitaOrizzontale -= 1;
		moving = true;
	}
	if (pressing_right)
	{
		VelocitaOrizzontale += 1;
		moving = true;
	}

	if (float_yoffset >= 0) {
		frame_animazione += 6;
		if (frame_animazione >= 360) {
			frame_animazione -= 360;
		}
	}

	if (!moving) {   

		if (VelocitaOrizzontale > 0)
		{
			VelocitaOrizzontale -= 1;
			if (VelocitaOrizzontale < 0)
				VelocitaOrizzontale = 0;
		}
		if (VelocitaOrizzontale < 0)
		{
			VelocitaOrizzontale += 1;
			if (VelocitaOrizzontale > 0)
				VelocitaOrizzontale = 0;
		}
	}
	
	//Aggioramento della posizione in x della navicella, che regola il movimento orizzontale
	posx += VelocitaOrizzontale;
	// Gestione Bordi viewport:
	// Se la navicella assume una posizione in x minore di 0 o maggiore di width dello schermo
	// facciamo rimbalzare la navicella ai bordi dello schermo
	if (posx < 0) {
		posx = 0;
		VelocitaOrizzontale = -VelocitaOrizzontale * 0.8;
	}
	if (posx > width) {
		posx = width;
		VelocitaOrizzontale = -VelocitaOrizzontale * 0.8;
	}
	// calcolo y come somma dei seguenti contributi: 
	//            pos. di equilibrio + oscillazione periodica
	posy = posizione_di_equilibrio + sin(degtorad(frame_animazione)) * range_fluttuazione;
	angolo = cos(degtorad(frame_animazione)) * angle_offset - VelocitaOrizzontale * 1.3;

	glutPostRedisplay();
	glutTimerFunc(15, updateV, 0);
}

void keyboardPressedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case ' ':
		pressing_attack = true;
		updateProiettile(0);
		break;
	case 'a':
		pressing_left = true;
		break;
	case 'd':
		pressing_right = true;
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

void keyboardReleasedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case ' ':
		pressing_attack = false;
		break;
	case 'a':
		pressing_left = false;
		break;
	case 'd':
		pressing_right = false;
		break;
	default:
		break;
	}
}

/////////////////////////// Disegna geometria scena //////////////////////////////////////

void disegna_tentacoli(Point* Punti, vec4 color_top, vec4 color_bot)
{
	float alfa = 2, step=PI/4;
	Point P0;
	int i;
	P0.x = 0.0;	P0.y = 0.0;  // centro circonferenza
	int cont = 0;

	for (i = 0; i < 8 ;i++)//  per ciascuno degli 8 tentacoli
	{
		Punti[cont].x = cos(i*step);  // punto sulla circonferenza 
		Punti[cont].y = sin(i*step);
		Punti[cont].r = color_bot.r; Punti[cont].g = color_bot.g; Punti[cont].b = color_bot.b; Punti[cont].a = color_bot.a;
		Punti[cont + 1].x = lerp(P0.x, Punti[cont].x, alfa);  //punto fuori circonferenza
		Punti[cont + 1].y = lerp(P0.y, Punti[cont].y, alfa);
		Punti[cont + 1].r = color_top.r; Punti[cont + 1].g = color_top.g; Punti[cont + 1].b = color_top.b; Punti[cont + 1].a = color_top.a;
		cont += 2;
	}
}

void disegna_piano(float x, float y, float width, float height, vec4 color_top, vec4 color_bot, Point* piano)
{
	piano[0].x = x;	piano[0].y = y; 
	piano[0].r = color_bot.r; piano[0].g = color_bot.g; piano[0].b = color_bot.b; piano[0].a = color_bot.a;
	piano[1].x = x + width;	piano[1].y = y;	
	piano[1].r = color_top.r; piano[1].g = color_top.g; piano[1].b = color_top.b; piano[1].a = color_top.a;
	piano[2].x = x + width;	piano[2].y = y + height; 
	piano[2].r = color_bot.r; piano[2].g = color_bot.g; piano[2].b = color_bot.b; piano[2].a = color_bot.a;

	piano[3].x = x + width;	piano[3].y = y + height; 
	piano[3].r = color_bot.r; piano[3].g = color_bot.g; piano[3].b = color_bot.b; piano[3].a = color_bot.a;
	piano[4].x = x;	piano[4].y = y + height; 
	piano[4].r = color_top.r; piano[4].g = color_top.g; piano[4].b = color_top.b; piano[4].a = color_top.a;
	piano[5].x = x;	piano[5].y = y; 
	piano[5].r = color_bot.r; piano[5].g = color_bot.g; piano[5].b = color_bot.b; piano[5].a = color_bot.a;
}

void disegna_cerchio(float cx, float cy, float raggiox, float raggioy, vec4 color_top, vec4 color_bot, Point* Cerchio)
{
	float stepA = (2 * PI) / nTriangles;

	int comp = 0;
	for (int i = 0; i < nTriangles; i++)
	{
		Cerchio[comp].x = cx + cos((double)i * stepA) * raggiox;
		Cerchio[comp].y = cy + sin((double)i * stepA) * raggioy;
		Cerchio[comp].r = color_top.r; Cerchio[comp].g = color_top.g; Cerchio[comp].b = color_top.b; Cerchio[comp].a = color_top.a;

		Cerchio[comp + 1].x = cx + cos((double)(i + 1) * stepA) * raggiox;
		Cerchio[comp + 1].y = cy + sin((double)(i + 1) * stepA) * raggioy;
		Cerchio[comp + 1].r = color_top.r; Cerchio[comp + 1].g = color_top.g; Cerchio[comp + 1].b = color_top.b; Cerchio[comp + 1].a = color_top.a;

		Cerchio[comp + 2].x = cx;
		Cerchio[comp + 2].y = cy;
		Cerchio[comp + 2].r = color_bot.r; Cerchio[comp + 2].g = color_bot.g; Cerchio[comp + 2].b = color_bot.b; Cerchio[comp + 2].a = color_bot.a;

		comp += 3;
	}
}

void Parte_superiore_navicella(float cx, float cy, float raggiox, float raggioy, vec4 color_top, vec4 color_bot, Point* Cerchio) {
	int i;
	int comp = 0; 
	float A = PI / 4;
	float B = 3 / 4 * PI;
	// arco tra A e B dato da PI/2
	float stepA = (PI/2)  / nTriangles;

	for (i = 0; i < nTriangles ; i++)
		{
		Cerchio[comp].x = cx + cos(A+(double)i * stepA) * raggiox;
		Cerchio[comp].y = cy + sin(A+(double)i * stepA) * raggioy;
		Cerchio[comp].r = color_top.r; Cerchio[comp].g = color_top.g; Cerchio[comp].b = color_top.b; Cerchio[comp].a = color_top.a;
		Cerchio[comp + 1].x = cx + cos(A+(double)(i + 1) * stepA) * raggiox;
		Cerchio[comp + 1].y = cy + sin(A+(double)(i + 1) * stepA) * raggioy;
		Cerchio[comp + 1].r = color_top.r; Cerchio[comp + 1].g = color_top.g; Cerchio[comp + 1].b = color_top.b; Cerchio[comp + 1].a = color_top.a;
		Cerchio[comp + 2].x =cx;
		Cerchio[comp + 2].y = cy;
		Cerchio[comp + 2].r = color_bot.r; Cerchio[comp + 2].g = color_bot.g; Cerchio[comp + 2].b = color_bot.b; Cerchio[comp + 2].a = color_bot.a;
		comp += 3;
	}
}

void disegna_Navicella(vec4 color_top_Navicella, vec4 color_bot_Navicella, vec4 color_top_corpo, vec4 color_bot_corpo, vec4 color_top_Oblo, vec4 color_bot_Oblo, Point* Navicella)
{
	int cont, i, v_Oblo;
	Point* Corpo;
	int v_Corpo =3 * nTriangles;
	Corpo = new Point[v_Corpo];

	Parte_superiore_navicella(0.0, -0.1, 1.0, 1.0, color_top_Navicella, color_bot_Navicella, Navicella);

	//Costruisci Corpo   cerchio + cintura + 3 bottoni 
	disegna_cerchio(0.0, -1.0, 1.0, 1.0, color_top_corpo, color_bot_corpo, Corpo);
	cont = 3 * nTriangles;

	for (i = 0; i < 3 * nTriangles; i++)
	{
		Navicella[i + cont].x = Corpo[i].x;
		Navicella[i + cont].y = Corpo[i].y;
		Navicella[i + cont].r = Corpo[i].r;	Navicella[i + cont].g = Corpo[i].g;	Navicella[i + cont].b = Corpo[i].b;	Navicella[i + cont].a = Corpo[i].a;
	}
	//Costruisci Corpo 2
	disegna_cerchio(1.0, -1.0, 0.5, 0.5, color_top_Oblo, color_bot_Oblo, Corpo);
	cont = 6 * nTriangles;
	for (i = 0; i < 3 * nTriangles; i++)
	{
		Navicella[i + cont].x = Corpo[i].x;
		Navicella[i + cont].y = Corpo[i].y;
		Navicella[i + cont].r = Corpo[i].r;	Navicella[i + cont].g = Corpo[i].g;	Navicella[i + cont].b = Corpo[i].b;	Navicella[i + cont].a = Corpo[i].a;
	}
    //Costruisci Corpo 3
	disegna_cerchio(-1.0, -1.0, 0.5, 0.5, color_top_Oblo, color_bot_Oblo, Corpo);

	cont = 9 * nTriangles;
	for (i = 0; i < 3 * nTriangles; i++)
	{
		Navicella[i + cont].x = Corpo[i].x;
		Navicella[i + cont].y = Corpo[i].y;
		Navicella[i + cont].r = Corpo[i].r;	Navicella[i + cont].g = Corpo[i].g;	Navicella[i + cont].b = Corpo[i].b;	Navicella[i + cont].a = Corpo[i].a;
	}
	cont = 12 * nTriangles;

//Proiettile
	Navicella[cont].x = 0;
	Navicella[cont].y = 0;
	Navicella[cont].r = 1;	Navicella[cont].g = 1;	Navicella[cont].b = 1;	Navicella[cont].a = 1;
}

void disegna_nemico(vec4 color_top_Nemico, vec4 color_bot_Nemico, vec4 color_top_Occhio, vec4 color_bot_Occhio, Point* Nemico)
{
	int i, cont;
	int v_faccia = 3 * nTriangles;
	int nV_Tentacoli = 16;
	Point* Occhio = new Point[v_faccia];
	Point* Tentacoli = new Point[nV_Tentacoli];

	// Disegna faccia del Nemico
	disegna_cerchio(0.0, 0.0, 1.0, 1.0, color_top_Nemico, color_bot_Nemico, Nemico);
	
	// Disegna i due occhi
	disegna_cerchio(-0.5, 0.5, 0.1, 0.1, color_top_Occhio, color_bot_Occhio, Occhio);
		cont = 3 * nTriangles;
	for (i = 0; i < v_faccia; i++)
		Nemico[i + cont] = Occhio[i];
	disegna_cerchio(0.5, 0.5, 0.1, 0.1, color_top_Occhio, color_bot_Occhio, Occhio);
		cont = cont + 3 * nTriangles;
	for (i = 0; i < v_faccia; i++)
		Nemico[i + cont] = Occhio[i];

	cont = cont + 3 * nTriangles;

	//Aggiungi bocca
	Nemico[cont].x = -0.5;	Nemico[cont].y = -0.5;	
	Nemico[cont].r = col_nero.r;Nemico[cont].g = col_nero.g;Nemico[cont].b = col_nero.b;Nemico[cont].a = col_nero.a;

	Nemico[cont + 1].x = -0.25;	Nemico[cont + 1].y = -0.25;	
	Nemico[cont + 1].r = col_nero.r; Nemico[cont + 1].g = col_nero.g; Nemico[cont + 1].b = col_nero.b; Nemico[cont + 1].a = col_nero.a;

	Nemico[cont + 2].x = 0.0;	Nemico[cont + 2].y = -0.5;	
	Nemico[cont + 2].r = col_nero.r; Nemico[cont + 2].g = col_nero.g; Nemico[cont + 2].b = col_nero.b; Nemico[cont + 2].a = col_nero.a;

	Nemico[cont + 3].x = 0.25;	Nemico[cont + 3].y = -0.25;
	Nemico[cont + 3].r = col_nero.r; Nemico[cont + 3].g = col_nero.g; Nemico[cont + 3].b = col_nero.b; Nemico[cont + 3].a = col_nero.a;

	Nemico[cont + 4].x = 0.5;Nemico[cont + 4].y = -0.5;	
	Nemico[cont + 4].r = col_nero.r; Nemico[cont + 4].g = col_nero.g; Nemico[cont + 4].b = col_nero.b; Nemico[cont + 4].a = col_nero.a;

	cont = cont + 5;
	
	disegna_tentacoli(Tentacoli, col_nero, col_rosso);
	for (i = 0; i < nV_Tentacoli; i++)
		Nemico[cont + i] = Tentacoli[i];
}

void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader_C_M.glsl";
	char* fragmentShader = (char*)"fragmentShader_C_M.glsl";
	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);
}

void init(void)
{
	
	//Disegno SPAZIO
	vec4 col_top = {0.1,0.1,0.1, 0.7};
	disegna_piano(0.0, 0.0, width, height, col_nero, col_top, Space);
	glGenVertexArrays(1, &VAO_SPACE);
	glBindVertexArray(VAO_SPACE);
	glGenBuffers(1, &VBO_C);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
	glBufferData(GL_ARRAY_BUFFER, vertices_space * sizeof(Point), &Space[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	
	//Disegno NAVICELLA
	vec4 color_top_Navicella = { 1.0,1.5,0.0,1.0 };
	vec4 color_bot_Navicella = { 1.0,0.8,0.8,0.5 };
	vec4 color_top_Corpo     = { 0.0,0.5,0.8,1.0 };
	vec4 color_bot_Corpo     = { 0.0,0.2,0.5,1.0 };
	vec4 color_top_Oblo      = { 0.2,0.9,0.1,1.0 };
	vec4 color_bot_Oblo      = { 0.0,0.2,0.8,1.0 };
	
	disegna_Navicella(color_top_Navicella, color_bot_Navicella, color_top_Corpo, color_bot_Corpo, color_top_Oblo, color_bot_Oblo, Navicella);
	//Genero un VAO navicella
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, nVertices_Navicella * sizeof(Point), &Navicella[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//Scollego il VAO
	glBindVertexArray(0);
	
	//Disegna nemici
	disegna_nemico(col_rosso, col_bianco, col_bianco, col_rosso, Nemico);
	glGenVertexArrays(1, &VAO_NEMICO);
	glBindVertexArray(VAO_NEMICO);
	glGenBuffers(1, &VBO_N);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_N);
	glBufferData(GL_ARRAY_BUFFER, nVertices_Nemico * sizeof(Point), &Nemico[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	
	colpito = new bool* [numero_di_righe];
	for (int i = 0; i < numero_di_righe; i++)
		colpito[i] = new bool[nemici_per_riga];
	for (int i = 0; i < numero_di_righe; i++)
		for (int j = 0; j < nemici_per_riga; j++)
		{
			colpito[i][j] = false;
			printf("%s", colpito[i][j] ? "true \n" : "false \n");
		}
	//Definisco il colore che verrà assegnato allo schermo
	glClearColor(1.0, 0.5, 0.0, 1.0);

	Projection = ortho(0.0f, float(width), 0.0f, float(height));
	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");
}

void drawScene(void)
{
	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programId);  

	// Disegna spazio
	glBindVertexArray(VAO_SPACE);
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glDrawArrays(GL_TRIANGLES, 0, vertices_space);
	glBindVertexArray(0);
	
	//Disegno Navicella e Proiettile
	glBindVertexArray(VAO);
	Model = mat4(1.0);
	Model = translate(Model, vec3(posx, posy, 0.0));
	Model = scale(Model, vec3(80.0, 40.0, 1.0));
	Model = rotate(Model, radians(angolo), vec3(0.0, 0.0, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glDrawArrays(GL_TRIANGLES, 0, nVertices_Navicella - 1); 
	//Disegno il proiettile
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPointSize(8.0);
	Model = mat4(1.0);
	Model = translate(Model, vec3(posx + posx_Proiettile, posy + posy_Proiettile, 0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glDrawArrays(GL_POINTS, nVertices_Navicella - 1, 1);
	glBindVertexArray(0);
	
	// Disegna nemici VIRUS
	glBindVertexArray(VAO_NEMICO);
	float passo_Nemici = ((float)width) / nemici_per_riga;
	float passo_righe = 150;
	for (int i = 0; i < numero_di_righe; i++)
	{
		posyN = height - i * passo_righe - 20;
		for (int j = 0; j < nemici_per_riga; j++)
		{
			posxN = j * (passo_Nemici)+passo_Nemici / 2;
			if (!colpito[i][j]) {
				Model = mat4(1.0);
				Model = translate(Model, vec3(posxN + dxnemici, posyN + dynemici, 0));
				Model = scale(Model, vec3(30.0, 30.0, 1.0));
				Model = rotate(Model, radians(angolo), vec3(0.0, 0.0, 1.0));
				glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				// faccia e occhi
				glDrawArrays(GL_TRIANGLES, 0, nVertices_Nemico - nvBocca - nvTentacoli);
				glLineWidth(3.0);  // bocca
				glDrawArrays(GL_LINE_STRIP, nVertices_Nemico - nvBocca - nvTentacoli, nvBocca);
				glLineWidth(4.0); // tentacoli
				glDrawArrays(GL_LINES, nVertices_Nemico - nvTentacoli, nvTentacoli);
			}
		}
	}
	glBindVertexArray(0);
	
	// calcolo collisione proiettile-nemico  -->  colpito
	for (int i = 0; i < numero_di_righe; i++)
	{
		posyN = height - i * passo_righe - 20 + rand() % 20;
		for (int j = 0; j < nemici_per_riga; j++)
		{
			posxN = j * (passo_Nemici)+passo_Nemici / 2 + rand() % 40;
			//	printf("Posizione del proiettile: x= %f y=%f \n", posx + posx_Proiettile, posy + posy_Proiettile);
			//	printf("Bounding Box (BB) nemico %d %d : xmin= %f ymin=%f  xmax=%f ymax=%f \n", i,j, posxN - 50 , posyN-50, + posx_Proiettile, posy + posy_Proiettile);
			if (((posx + posx_Proiettile >= posxN + dxnemici - 10) && (posx + posx_Proiettile <= posxN + dxnemici + 10)) && ((posy + posy_Proiettile >= posyN + dynemici - 10) && (posy + posy_Proiettile <= posyN + dynemici + 10)))
			{
				if (!colpito[i][j])
				{
					NumeroColpiti++;
					printf("Numero colpiti %d \n", NumeroColpiti);
					colpito[i][j] = true;
				}
			}
		}
	}
	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("2D Game COV-19");
	glutDisplayFunc(drawScene);
	//Evento tastiera tasto premuto
	glutKeyboardFunc(keyboardPressedEvent);
	//Evento tastiera tasto rilasciato
	glutKeyboardUpFunc(keyboardReleasedEvent);
	// glutTimerFunc a timer callback is triggered in a specified number of milliseconds
	glutTimerFunc(500, update, 0);
	glutTimerFunc(500, updateV, 0);
	glutTimerFunc(500, updateNemici, 0);

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutMainLoop();
}
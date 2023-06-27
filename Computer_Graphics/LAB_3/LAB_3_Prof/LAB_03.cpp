/******************************************************************************************
LAB 03
Gestione interattiva di una scena 3D mediante controllo da mouse e da tastiera.
I modelli geometrici in scena sono mesh poligonali in formato *.obj

TRL+WHEEL = pan orizzontale della telecamera
SHIFT+WHEEL = pan verticale della telecamera
WHEEL = se navigazione  --> ZOOM IN/OUT
		se modify       --> agisce sulla trasformazione dell'oggetto
g r s	per le modalità di lavoro: traslate/rotate/scale
x y z	per l'asse di lavoro
wcs/ocs selezionabili dal menu pop-up

OpenGL Mathematics (GLM) is a header only C++ mathematics library for graphics software
based on the OpenGL Shading Language (GLSL) specifications.
*******************************************************************************************/

#define _CRT_SECURE_NO_WARNINGS // for fscanf

#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include <ctime>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "HUD_Logger.h"
#include "ShaderMaker.h"

#define SHIFT_WHEEL_UP 11
#define SHIFT_WHEEL_DOWN 12
#define CTRL_WHEEL_UP 19
#define CTRL_WHEEL_DOWN 20

#define NUM_SHADERS 9

using namespace std;

// Viewport size
static int WindowWidth = 1120; //1366;
static int WindowHeight = 630; // 768;
GLfloat aspect_ratio = 16.0f / 9.0f;

typedef struct {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	GLuint vertexArrayObjID;
	GLuint vertexBufferObjID;
	GLuint normalBufferObjID;
	GLuint uvBufferObjID;
} Mesh;

typedef enum {
	RED_PLASTIC,
	CYAN_PLASTIC,
	EMERALD,
	BRASS,
	SLATE,
	SILVER,
	GOLD,
	NO_MATERIAL
} MaterialType;

typedef struct {
	std::string name;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	GLfloat shininess;
} Material;

typedef enum { // used also as index, don't modify order
	GOURAUD,
	PHONG,
	BLINN,
	TOON,
	PASS_THROUGH,
	WAVE,
	WAVE_PHONG
} ShadingType;

typedef struct {
	Mesh mesh;
	MaterialType material;
	ShadingType shading;
	glm::mat4 M;
	string name;
} Object;

typedef struct {
	GLuint light_position_pointer;
	GLuint light_color_pointer;
	GLuint light_power_pointer;
	GLuint material_diffuse;
	GLuint material_ambient;
	GLuint material_specular;
	GLuint material_shininess;
} LightShaderUniform;

typedef struct {
	GLuint P_Matrix_pointer;
	GLuint V_Matrix_pointer;
	GLuint M_Matrix_pointer;
	GLfloat time_delta_pointer;
} BaseShaderUniform;

const string MeshDir = "Mesh/";
const string ShaderDir = "Shaders/";
static Object Axis, Grid;
static vector<Object> objects;
static vector<Material> materials;
static int selected_obj = 0;

// Materiali disponibili
glm::vec3 red_plastic_ambient = { 0.1, 0.0, 0.0 }, red_plastic_diffuse = { 0.6, 0.1, 0.1 }, red_plastic_specular = { 0.7, 0.6, 0.6 }; GLfloat red_plastic_shininess = 32.0f;
glm::vec3 brass_ambient = { 0.1, 0.06, 0.015 }, brass_diffuse = { 0.78, 0.57, 0.11 }, brass_specular = { 0.99, 0.91, 0.81 }; GLfloat brass_shininess = 27.8f;
glm::vec3 emerald_ambient = { 0.0215, 0.04745, 0.0215 }, emerald_diffuse = { 0.07568, 0.61424, 0.07568 }, emerald_specular = { 0.633, 0.727811, 0.633 }; GLfloat emerald_shininess = 78.8f;
glm::vec3 slate_ambient = { 0.02, 0.02, 0.02 }, slate_diffuse = { 0.1, 0.1, 0.1 }, slate_specular{ 0.4, 0.4, 0.4 }; GLfloat slate_shininess = 1.78125f;
glm::vec3 silver_ambient = { 0.19225,0.19225, 0.19225 }, silver_diffuse = { 0.50754, 0.50754, 0.50754 }, silver_specular = { 0.508273, 0.508273, 0.508273 }; GLfloat silver_shininess = 0.4f;
glm::vec3 gold_ambient = { 0.24725, 0.1995, 0.0745 }, gold_diffuse = { 0.75164,0.60648,0.22648 }, gold_specular = { 0.628281, 0.555802, 0.366065 }; GLfloat golf_shininess = 0.4f;
glm::vec3 cyan_plastic_ambient = { 0.0, 0.1, 0.06 }, cyan_plastic_diffuse = { 0.0, 0.50980392f,0.50980392f }, cyan_plastic_specular = { 0.50196078f,0.50196078f,0.50196078f }; GLfloat cyan_plastic_shininess = 32.0f;

typedef struct {
	glm::vec3 position;
	glm::vec3 color;
	GLfloat power;
} point_light;

static point_light light;

/*camera structures*/
constexpr float CAMERA_ZOOM_SPEED = 0.1f;
constexpr float CAMERA_TRASLATION_SPEED = 0.01f;

struct {
	glm::vec4 position;
	glm::vec4 target;
	glm::vec4 upVector;
} ViewSetup;

struct {
	float fovY, aspect, near_plane, far_plane;
} PerspectiveSetup;

typedef enum {
	WIRE_FRAME,
	FACE_FILL,
	CULLING_ON,
	CULLING_OFF,
	CHANGE_TO_WCS,
	CHANGE_TO_OCS
} MenuOption;

enum {
	NAVIGATION,
	CAMERA_MOVING,
	TRASLATING,
	ROTATING,
	SCALING
} OperationMode;

enum {
	X,
	Y,
	Z
} WorkingAxis;

enum {
	OCS, // Object Coordinate System
	WCS // World Coordinate System
} TransformMode;

static bool moving_trackball = 0;
static int last_mouse_pos_Y;
static int last_mouse_pos_X;

//Shaders Uniforms 
static vector<LightShaderUniform> light_uniforms; // for shaders with light
static vector<BaseShaderUniform> base_uniforms; // for ALL shaders
static vector<GLuint> shaders_IDs; //Pointers to the shader programs
// Main initialization funtion
void init();
// Display Funtion
void display();
// Reshape Function
void resize(int w, int h);
// Calls glutPostRedisplay each millis milliseconds
void refresh_monitor(int millis);
// Mouse Function
void mouseClick(int button, int state, int x, int y);
// Keyboard:  g traslate r rotate s scale x,y,z axis esc 
void keyboardDown(unsigned char key, int x, int y);
// Special key arrow: select active object (arrows left,right)
void special(int key, int x, int y);
// gestione delle voci principali del menu
void main_menu_func(int option);
// gestione delle voci principali del sub menu per i matriali
void material_menu_function(int option);
// costruisce i menu openGL
void buildOpenGLMenu();
// Trackball: Converte un punto 2D sullo schermo in un punto 3D sulla trackball
glm::vec3 getTrackBallPoint(float x, float y);
// Trackball: Effettua la rotazione del vettore posizione sulla trackball
void mouseActiveMotion(int x, int y);
void moveCameraForeward();
void moveCameraBack();
void moveCameraLeft();
void moveCameraRight();
void moveCameraUp();
void moveCameraDown();
//	Crea ed applica la matrice di trasformazione alla matrice dell'oggetto discriminando tra WCS e OCS.
//	La funzione è gia invocata con un input corretto, è sufficiente concludere la sua implementazione.
void modifyModelMatrix(glm::vec3 translation_vector, glm::vec3 rotation_vector, GLfloat angle, GLfloat scale_factor);
/* Mesh Functions*/
// Genera i buffer per la mesh in input e ne salva i puntatori di openGL
void generate_and_load_buffers(bool generate, Mesh* mesh);
// legge un file obj ed inizializza i vector della mesh in input
void loadObjFile(string file_path, Mesh* mesh);
// disegna l'origine del assi
void drawAxisAndGrid();
// 2D fixed pipeline Font rendering on screen
void printToScreen();

void init_light_object() {
	Mesh sphereS = {};
	loadObjFile(MeshDir + "sphere_n_t_smooth.obj", &sphereS);
	generate_and_load_buffers(true, &sphereS);
	// Object Setup, use the light shader and a material for color and light behavior
	Object obj = {};
	obj.mesh = sphereS;
	obj.material = MaterialType::NO_MATERIAL;
	obj.shading = ShadingType::GOURAUD;
	obj.name = "light";
	obj.M = glm::scale(glm::translate(glm::mat4(1), light.position), glm::vec3(0.2, 0.2, 0.2));
	objects.push_back(obj);
}

void init_waving_plane() {
	Mesh sphereS = {};
	loadObjFile(MeshDir + "GridPlane.obj", &sphereS);
	generate_and_load_buffers(true, &sphereS);
	// Object Setup use the light shader and a material for color and light behavior
	Object obj4 = {};
	obj4.mesh = sphereS;
	obj4.material = MaterialType::CYAN_PLASTIC;
	//obj4.shading = ShadingType::GOURAUD;
	obj4.shading = ShadingType::WAVE;// WAVE;
	//obj4.shading = ShadingType::WAVE_PHONG;// WAVE_PHONG;
	obj4.name = "Waves";
	obj4.M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0., -2., 0.)), glm::vec3(8., 8., 8.));
	objects.push_back(obj4);
}

void init_mesh() {
	Mesh sphereS = {};
	//	loadObjFile(MeshDir + "airplane.obj", &sphereS);
	loadObjFile(MeshDir + "bunny.obj", &sphereS);
	generate_and_load_buffers(true, &sphereS);
	// Object Setup use the light shader and a material for color and light behavior
	Object obj4 = {};
	obj4.mesh = sphereS;
	obj4.material = MaterialType::RED_PLASTIC; // NO_MATERIAL;
	obj4.shading = ShadingType::TOON; // GOURAUD; // TOON;
	obj4.name = "Bunny";
	obj4.M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0., 0., -2.)), glm::vec3(2., 2., 2.));

	objects.push_back(obj4);
}


void init_sphere_FLAT() {
	Mesh sphereF = {};
	loadObjFile(MeshDir + "sphere_n_t_flat.obj", &sphereF);
	generate_and_load_buffers(true, &sphereF);
	// Object Setup  use the light shader and a material for color and light behavior
	Object obj3 = {};
	obj3.mesh = sphereF;
	obj3.material = MaterialType::EMERALD;
	obj3.shading = ShadingType::PHONG;
	obj3.name = "Sphere FLAT";
	obj3.M = glm::translate(glm::mat4(1), glm::vec3(3., 0., -6.));
	objects.push_back(obj3);
}

void init_sphere_SMOOTH() {
	Mesh sphereS = {};
	loadObjFile(MeshDir + "sphere_n_t_smooth.obj", &sphereS);
	generate_and_load_buffers(true, &sphereS);
	// Object Setup use the light shader and a material for color and light behavior
	Object obj4 = {};
	obj4.mesh = sphereS;
	obj4.material = MaterialType::EMERALD;
	obj4.shading = ShadingType::BLINN;
	obj4.name = "Sphere SMOOTH";
	obj4.M = glm::translate(glm::mat4(1), glm::vec3(6., 0., -3.));
	objects.push_back(obj4);
}

void init_axis() {
	Mesh _grid = {};
	loadObjFile(MeshDir + "axis.obj", &_grid);
	generate_and_load_buffers(true, &_grid);
	Object obj1 = {};
	obj1.mesh = _grid;
	obj1.material = MaterialType::NO_MATERIAL;
	obj1.shading = ShadingType::BLINN;
	obj1.name = "axis_";
	obj1.M = glm::scale(glm::mat4(1), glm::vec3(2.f, 2.f, 2.f));
	Axis = obj1;
}

void init_grid() {
	Mesh _grid = {};
	loadObjFile(MeshDir + "reference_grid.obj", &_grid);
	generate_and_load_buffers(true, &_grid);
	Object obj1 = {};
	obj1.mesh = _grid;
	obj1.material = MaterialType::NO_MATERIAL;
	obj1.shading = ShadingType::PASS_THROUGH;
	obj1.name = "grid_";
	obj1.M = glm::mat4(1);
	Grid = obj1;
}


void initShader()
{
	// SHADERS configuration section
	shaders_IDs.resize(NUM_SHADERS);
	light_uniforms.resize(NUM_SHADERS); // allocate space for uniforms of PHONG, BLINN and GOURAND + TOON
	base_uniforms.resize(NUM_SHADERS); // allocate space for uniforms of PHONG,BLINN,GOURAND,TOON,WAVE

	//Gourand Shader loading
	shaders_IDs[GOURAUD] = createProgram(ShaderDir + "v_gouraud.glsl", ShaderDir + "f_gouraud.glsl");
	BaseShaderUniform base_unif = {};
	//Otteniamo i puntatori alle variabili uniform per poterle utilizzare in seguito
	base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[GOURAUD], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[GOURAUD], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[GOURAUD], "M");
	base_uniforms[ShadingType::GOURAUD] = base_unif;
	LightShaderUniform light_unif = {};
	light_unif.material_ambient = glGetUniformLocation(shaders_IDs[GOURAUD], "material.ambient");
	light_unif.material_diffuse = glGetUniformLocation(shaders_IDs[GOURAUD], "material.diffuse");
	light_unif.material_specular = glGetUniformLocation(shaders_IDs[GOURAUD], "material.specular");
	light_unif.material_shininess = glGetUniformLocation(shaders_IDs[GOURAUD], "material.shininess");
	light_unif.light_position_pointer = glGetUniformLocation(shaders_IDs[GOURAUD], "light.position");
	light_unif.light_color_pointer = glGetUniformLocation(shaders_IDs[GOURAUD], "light.color");
	light_unif.light_power_pointer = glGetUniformLocation(shaders_IDs[GOURAUD], "light.power");
	light_uniforms[ShadingType::GOURAUD] = light_unif;
	//Rendiamo attivo lo shader
	glUseProgram(shaders_IDs[GOURAUD]);
	//Shader uniforms initialization
	glUniform3f(light_uniforms[GOURAUD].light_position_pointer, light.position.x, light.position.y, light.position.z);
	glUniform3f(light_uniforms[GOURAUD].light_color_pointer, light.color.r, light.color.g, light.color.b);
	glUniform1f(light_uniforms[GOURAUD].light_power_pointer, light.power);

	//Phong Shader loading
	shaders_IDs[PHONG] = createProgram(ShaderDir + "v_phong.glsl", ShaderDir + "f_phong.glsl");
	//Otteniamo i puntatori alle variabili uniform per poterle utilizzare in seguito
	base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[PHONG], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[PHONG], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[PHONG], "M");
	base_uniforms[ShadingType::PHONG] = base_unif;
	light_unif.material_ambient = glGetUniformLocation(shaders_IDs[PHONG], "material.ambient");
	light_unif.material_diffuse = glGetUniformLocation(shaders_IDs[PHONG], "material.diffuse");
	light_unif.material_specular = glGetUniformLocation(shaders_IDs[PHONG], "material.specular");
	light_unif.material_shininess = glGetUniformLocation(shaders_IDs[PHONG], "material.shininess");
	light_unif.light_position_pointer = glGetUniformLocation(shaders_IDs[PHONG], "light.position");
	light_unif.light_color_pointer = glGetUniformLocation(shaders_IDs[PHONG], "light.color");
	light_unif.light_power_pointer = glGetUniformLocation(shaders_IDs[PHONG], "light.power");
	light_uniforms[ShadingType::PHONG] = light_unif;
	//Rendiamo attivo lo shader
	glUseProgram(shaders_IDs[PHONG]);
	//Shader uniforms initialization
	glUniform3f(light_uniforms[PHONG].light_position_pointer, light.position.x, light.position.y, light.position.z);
	glUniform3f(light_uniforms[PHONG].light_color_pointer, light.color.r, light.color.g, light.color.b);
	glUniform1f(light_uniforms[PHONG].light_power_pointer, light.power);

	//Blinn Shader loading
	shaders_IDs[BLINN] = createProgram(ShaderDir + "v_blinn.glsl", ShaderDir + "f_blinn.glsl");
	base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[BLINN], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[BLINN], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[BLINN], "M");
	base_uniforms[ShadingType::BLINN] = base_unif;
	light_unif.material_ambient = glGetUniformLocation(shaders_IDs[BLINN], "material.ambient");
	light_unif.material_diffuse = glGetUniformLocation(shaders_IDs[BLINN], "material.diffuse");
	light_unif.material_specular = glGetUniformLocation(shaders_IDs[BLINN], "material.specular");
	light_unif.material_shininess = glGetUniformLocation(shaders_IDs[BLINN], "material.shininess");
	light_unif.light_position_pointer = glGetUniformLocation(shaders_IDs[BLINN], "light.position");
	light_unif.light_color_pointer = glGetUniformLocation(shaders_IDs[BLINN], "light.color");
	light_unif.light_power_pointer = glGetUniformLocation(shaders_IDs[BLINN], "light.power");
	light_uniforms[ShadingType::BLINN] = light_unif;
	//Rendiamo attivo lo shader
	glUseProgram(shaders_IDs[BLINN]);
	//Shader uniforms initialization
	glUniform3f(light_uniforms[BLINN].light_position_pointer, light.position.x, light.position.y, light.position.z);
	glUniform3f(light_uniforms[BLINN].light_color_pointer, light.color.r, light.color.g, light.color.b);
	glUniform1f(light_uniforms[BLINN].light_power_pointer, light.power);

	//Wave Shader Loading
	//TODO
	shaders_IDs[WAVE] = createProgram(ShaderDir + "v_wave.glsl", ShaderDir + "f_gouraud.glsl");
	base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[WAVE], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[WAVE], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[WAVE], "M");
	base_unif.time_delta_pointer = glGetUniformLocation(shaders_IDs[WAVE], "time");
	base_uniforms[ShadingType::WAVE] = base_unif;
	light_unif.material_ambient = glGetUniformLocation(shaders_IDs[WAVE], "material.ambient");
	light_unif.material_diffuse = glGetUniformLocation(shaders_IDs[WAVE], "material.diffuse");
	light_unif.material_specular = glGetUniformLocation(shaders_IDs[WAVE], "material.specular");
	light_unif.material_shininess = glGetUniformLocation(shaders_IDs[WAVE], "material.shininess");
	light_unif.light_position_pointer = glGetUniformLocation(shaders_IDs[WAVE], "light.position");
	light_unif.light_color_pointer = glGetUniformLocation(shaders_IDs[WAVE], "light.color");
	light_unif.light_power_pointer = glGetUniformLocation(shaders_IDs[WAVE], "light.power");
	light_uniforms[ShadingType::WAVE] = light_unif;
	//Rendiamo attivo lo shader
	glUseProgram(shaders_IDs[WAVE]);
	//Shader uniforms initialization
	glUniform3f(light_uniforms[WAVE].light_position_pointer, light.position.x, light.position.y, light.position.z);
	glUniform3f(light_uniforms[WAVE].light_color_pointer, light.color.r, light.color.g, light.color.b);
	glUniform1f(light_uniforms[WAVE].light_power_pointer, light.power);

	//Wave Phong Shader Loading
	shaders_IDs[PHONG] = createProgram(ShaderDir + "v_wave_phong.glsl", ShaderDir + "f_phong.glsl");
	//Otteniamo i puntatori alle variabili uniform per poterle utilizzare in seguito
	base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "M");
	base_unif.time_delta_pointer = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "time");
	base_uniforms[ShadingType::PHONG] = base_unif;
	light_unif.material_ambient = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "material.ambient");
	light_unif.material_diffuse = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "material.diffuse");
	light_unif.material_specular = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "material.specular");
	light_unif.material_shininess = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "material.shininess");
	light_unif.light_position_pointer = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "light.position");
	light_unif.light_color_pointer = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "light.color");
	light_unif.light_power_pointer = glGetUniformLocation(shaders_IDs[WAVE_PHONG], "light.power");
	light_uniforms[ShadingType::WAVE_PHONG] = light_unif;
	//Rendiamo attivo lo shader
	glUseProgram(shaders_IDs[WAVE_PHONG]);
	//Shader uniforms initialization
	glUniform3f(light_uniforms[WAVE_PHONG].light_position_pointer, light.position.x, light.position.y, light.position.z);
	glUniform3f(light_uniforms[WAVE_PHONG].light_color_pointer, light.color.r, light.color.g, light.color.b);
	glUniform1f(light_uniforms[WAVE_PHONG].light_power_pointer, light.power);

	//TOON Shader Loading
	//TODO
	shaders_IDs[TOON] = createProgram(ShaderDir + "v_toon.glsl", ShaderDir + "f_toon.glsl");
	//Otteniamo i puntatori alle variabili uniform per poterle utilizzare in seguito
	base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[TOON], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[TOON], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[TOON], "M");
	base_uniforms[ShadingType::TOON] = base_unif;
	light_unif.light_position_pointer = glGetUniformLocation(shaders_IDs[TOON], "lightPosition");
	light_uniforms[ShadingType::TOON] = light_unif;
	//Rendiamo attivo lo shader
	glUseProgram(shaders_IDs[TOON]);
	//Shader uniforms initialization
	glUniform3f(light_uniforms[TOON].light_position_pointer, light.position.x, light.position.y, light.position.z);
	glUniform4fv(glGetUniformLocation(shaders_IDs[TOON], "Color"), 1, value_ptr(glm::vec4(1.0, 1.0, 1.0, 1.0)));
}

void init() {
	// Default render settings
	OperationMode = NAVIGATION;
	glEnable(GL_DEPTH_TEST);	// Hidden surface removal
	glCullFace(GL_BACK);	// remove faces facing the background
	glEnable(GL_LINE_SMOOTH);

	//Light initialization
	light.position = { 5.0,5.0,-5.0 };
	light.color = { 1.0,1.0,1.0 };
	light.power = 1.f;

	// Materials setup
	materials.resize(8);
	materials[MaterialType::RED_PLASTIC].name = "Red Plastic";
	materials[MaterialType::RED_PLASTIC].ambient = red_plastic_ambient;
	materials[MaterialType::RED_PLASTIC].diffuse = red_plastic_diffuse;
	materials[MaterialType::RED_PLASTIC].specular = red_plastic_specular;
	materials[MaterialType::RED_PLASTIC].shininess = red_plastic_shininess;

	materials[MaterialType::EMERALD].name = "Emerald";
	materials[MaterialType::EMERALD].ambient = emerald_ambient;
	materials[MaterialType::EMERALD].diffuse = emerald_diffuse;
	materials[MaterialType::EMERALD].specular = emerald_specular;
	materials[MaterialType::EMERALD].shininess = emerald_shininess;

	materials[MaterialType::BRASS].name = "Brass";
	materials[MaterialType::BRASS].ambient = brass_ambient;
	materials[MaterialType::BRASS].diffuse = brass_diffuse;
	materials[MaterialType::BRASS].specular = brass_specular;
	materials[MaterialType::BRASS].shininess = brass_shininess;

	materials[MaterialType::SLATE].name = "Slate";
	materials[MaterialType::SLATE].ambient = slate_ambient;
	materials[MaterialType::SLATE].diffuse = slate_diffuse;
	materials[MaterialType::SLATE].specular = slate_specular;
	materials[MaterialType::SLATE].shininess = slate_shininess;

	materials[MaterialType::SILVER].name = "Silver";
	materials[MaterialType::SILVER].ambient = silver_ambient;
	materials[MaterialType::SILVER].diffuse = silver_diffuse;
	materials[MaterialType::SILVER].specular = silver_specular;
	materials[MaterialType::SILVER].shininess = silver_shininess;

	materials[MaterialType::GOLD].name = "Gold";
	materials[MaterialType::GOLD].ambient = gold_ambient;
	materials[MaterialType::GOLD].diffuse = gold_diffuse;
	materials[MaterialType::GOLD].specular = gold_specular;
	materials[MaterialType::GOLD].shininess = golf_shininess;

	materials[MaterialType::CYAN_PLASTIC].name = "Cyan Plastic";
	materials[MaterialType::CYAN_PLASTIC].ambient = cyan_plastic_ambient;
	materials[MaterialType::CYAN_PLASTIC].diffuse = cyan_plastic_diffuse;
	materials[MaterialType::CYAN_PLASTIC].specular = cyan_plastic_specular;
	materials[MaterialType::CYAN_PLASTIC].shininess = cyan_plastic_shininess;

	materials[MaterialType::NO_MATERIAL].name = "NO_MATERIAL";
	materials[MaterialType::NO_MATERIAL].ambient = glm::vec3(1, 1, 1);
	materials[MaterialType::NO_MATERIAL].diffuse = glm::vec3(0, 0, 0);
	materials[MaterialType::NO_MATERIAL].specular = glm::vec3(0, 0, 0);
	materials[MaterialType::NO_MATERIAL].shininess = 1.f;

	// Camera Setup
	ViewSetup = {};
	ViewSetup.position = glm::vec4(10.0, 10.0, 10.0, 0.0);
	ViewSetup.target = glm::vec4(0.0, 0.0, 0.0, 0.0);
	ViewSetup.upVector = glm::vec4(0.0, 1.0, 0.0, 0.0);
	PerspectiveSetup = {};
	PerspectiveSetup.aspect = (GLfloat)WindowWidth / (GLfloat)WindowHeight;
	PerspectiveSetup.fovY = 45.0f;
	PerspectiveSetup.far_plane = 2000.0f;
	PerspectiveSetup.near_plane = 1.0f;

	//////////////////////////////////////////////////////////////////////
	//				OBJECTS IN SCENE
	//////////////////////////////////////////////////////////////////////
	// FLAT SPHERE (face normals) uses a shader with lighting
	init_sphere_FLAT();

	// SMOOTH SPHERE (vertex normals)  uses a shader with lighting
	init_sphere_SMOOTH();

	//Reference point of the position of the light
	init_light_object();

	// White Axis for reference
	init_axis();

	// White Grid for reference
	init_grid();

	// Waving plane
	init_waving_plane();

	//Airplane model with TOON SHADING
	init_mesh();
}

void drawScene() {

	glClearColor(0.4, 0.4, 0.4, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	drawAxisAndGrid(); // The central Axis point of reference

	for (int i = 0; i < objects.size(); i++) {
		//Shader selection
		switch (objects[i].shading) {
		case ShadingType::GOURAUD:
			glUseProgram(shaders_IDs[GOURAUD]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[GOURAUD].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			//Material loading
			glUniform3fv(light_uniforms[GOURAUD].material_ambient, 1, glm::value_ptr(materials[objects[i].material].ambient));
			glUniform3fv(light_uniforms[GOURAUD].material_diffuse, 1, glm::value_ptr(materials[objects[i].material].diffuse));
			glUniform3fv(light_uniforms[GOURAUD].material_specular, 1, glm::value_ptr(materials[objects[i].material].specular));
			glUniform1f(light_uniforms[GOURAUD].material_shininess, materials[objects[i].material].shininess);
			break;
		case ShadingType::PHONG:
			glUseProgram(shaders_IDs[PHONG]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[PHONG].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			//Material loading
			glUniform3fv(light_uniforms[PHONG].material_ambient, 1, glm::value_ptr(materials[objects[i].material].ambient));
			glUniform3fv(light_uniforms[PHONG].material_diffuse, 1, glm::value_ptr(materials[objects[i].material].diffuse));
			glUniform3fv(light_uniforms[PHONG].material_specular, 1, glm::value_ptr(materials[objects[i].material].specular));
			glUniform1f(light_uniforms[PHONG].material_shininess, materials[objects[i].material].shininess);
			break;
		case ShadingType::BLINN:
			glUseProgram(shaders_IDs[BLINN]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[BLINN].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			//Material loading
			glUniform3fv(light_uniforms[BLINN].material_ambient, 1, glm::value_ptr(materials[objects[i].material].ambient));
			glUniform3fv(light_uniforms[BLINN].material_diffuse, 1, glm::value_ptr(materials[objects[i].material].diffuse));
			glUniform3fv(light_uniforms[BLINN].material_specular, 1, glm::value_ptr(materials[objects[i].material].specular));
			glUniform1f(light_uniforms[BLINN].material_shininess, materials[objects[i].material].shininess);
			break;
		case ShadingType::TOON:
			glUseProgram(shaders_IDs[TOON]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[TOON].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			break;
		case ShadingType::PASS_THROUGH:
			glUseProgram(shaders_IDs[PASS_THROUGH]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[PASS_THROUGH].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			break;
		case ShadingType::WAVE:
			glUseProgram(shaders_IDs[WAVE]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[WAVE].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			// Time setting
			glUniform1f(base_uniforms[WAVE].time_delta_pointer, clock());
			//Material loading
			glUniform3fv(light_uniforms[GOURAUD].material_ambient, 1, glm::value_ptr(materials[objects[i].material].ambient));
			glUniform3fv(light_uniforms[GOURAUD].material_diffuse, 1, glm::value_ptr(materials[objects[i].material].diffuse));
			glUniform3fv(light_uniforms[GOURAUD].material_specular, 1, glm::value_ptr(materials[objects[i].material].specular));
			glUniform1f(light_uniforms[GOURAUD].material_shininess, materials[objects[i].material].shininess);
			break;
		case ShadingType::WAVE_PHONG:
			glUseProgram(shaders_IDs[WAVE_PHONG]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[WAVE_PHONG].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			// Time setting
			glUniform1f(base_uniforms[WAVE_PHONG].time_delta_pointer, clock());
			//Material loading
			glUniform3fv(light_uniforms[WAVE_PHONG].material_ambient, 1, glm::value_ptr(materials[objects[i].material].ambient));
			glUniform3fv(light_uniforms[WAVE_PHONG].material_diffuse, 1, glm::value_ptr(materials[objects[i].material].diffuse));
			glUniform3fv(light_uniforms[WAVE_PHONG].material_specular, 1, glm::value_ptr(materials[objects[i].material].specular));
			glUniform1f(light_uniforms[WAVE_PHONG].material_shininess, materials[objects[i].material].shininess);
			break;
		default:
			break;
		}

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindVertexArray(objects[i].mesh.vertexArrayObjID);
		glDrawArrays(GL_TRIANGLES, 0, objects[i].mesh.vertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}


	// OLD fixed pipeline for simple graphics and symbols
	glUseProgram(0);
	printToScreen();

	glutSwapBuffers();
}

void resize(int w, int h)
{
	if (h == 0)	// Window is minimized
		return;
	int width = h * aspect_ratio;           // width is adjusted for aspect ratio
	int left = (w - width) / 2;
	// Set Viewport to window dimensions
	glViewport(left, 0, width, h);
	WindowWidth = w;
	WindowHeight = h;

	// Fixed Pipeline matrices for retro compatibility
	glUseProgram(0); // Embedded openGL shader
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(PerspectiveSetup.fovY, PerspectiveSetup.aspect, PerspectiveSetup.near_plane, PerspectiveSetup.far_plane);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(ViewSetup.position.x, ViewSetup.position.y, ViewSetup.position.z,
		ViewSetup.target.x, ViewSetup.target.y, ViewSetup.target.z,
		ViewSetup.upVector.x, ViewSetup.upVector.y, ViewSetup.upVector.z);

	// Programmable Pipeline matrices for object rendering
	glm::mat4 P = glm::perspective(PerspectiveSetup.fovY, PerspectiveSetup.aspect, PerspectiveSetup.near_plane, PerspectiveSetup.far_plane);
	glm::mat4 V = glm::lookAt(glm::vec3(ViewSetup.position), glm::vec3(ViewSetup.target), glm::vec3(ViewSetup.upVector));

	for (int i = 0; i < shaders_IDs.size(); i++) {
		glUseProgram(shaders_IDs[i]);
		glUniformMatrix4fv(base_uniforms[i].P_Matrix_pointer, 1, GL_FALSE, value_ptr(P));
		glUniformMatrix4fv(base_uniforms[i].V_Matrix_pointer, 1, GL_FALSE, value_ptr(V));
	}
}

void refresh_monitor(int millis)
{
	glutPostRedisplay();
	glutTimerFunc(millis, refresh_monitor, millis);
}

void mouseClick(int button, int state, int x, int y)
{
	glutPostRedisplay();
	int modifiers = glutGetModifiers();
	if (modifiers == GLUT_ACTIVE_SHIFT) {
		switch (button)
		{
		case SHIFT_WHEEL_UP: moveCameraUp(); break;
		case SHIFT_WHEEL_DOWN: moveCameraDown(); break;
		}
		return;
	}
	if (modifiers == GLUT_ACTIVE_CTRL) {
		switch (button)
		{
		case CTRL_WHEEL_UP: moveCameraRight(); break;
		case CTRL_WHEEL_DOWN: moveCameraLeft(); break;
		}
		return;
	}

	glm::vec4 axis;
	float amount = 0.10f;
	// Imposto il valore della trasformazione
	switch (button) {
	case 3:// scroll wheel up
		amount *= 1;
		break;
	case 4:// scroll wheel down
		amount *= -1;
		break;
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) { moving_trackball = true; }
		if (state == GLUT_UP) { moving_trackball = false; }
		OperationMode = NAVIGATION;
		last_mouse_pos_X = x;
		last_mouse_pos_Y = y;
		break;
	default:
		break;
	}

	// Selezione dell'asse per le trasformazioni
	switch (WorkingAxis) {
	case X:	axis = glm::vec4(1.0, 0.0, 0.0, 0.0);
		break;
	case Y: axis = glm::vec4(0.0, 1.0, 0.0, 0.0);
		break;
	case Z: axis = glm::vec4(0.0, 0.0, 1.0, 0.0);
		break;
	default:
		break;
	}

	switch (OperationMode) {
	case TRASLATING:
		modifyModelMatrix(axis * amount, axis, 0.0f, 1.0f);
		break;
	case ROTATING:
		modifyModelMatrix(glm::vec3(0), axis, amount * 20.0f, 1.0f);
		break;
	case SCALING:
		modifyModelMatrix(glm::vec3(0), axis, 0.0f, 1.0f + amount);
		break;
	case NAVIGATION:
		// Wheel reports as button 3(scroll up) and button 4(scroll down)
		if (button == 3) {
			moveCameraBack();
		}
		else if (button == 4) {
			moveCameraForeward();
		}
		break;
	default:
		break;
	}
}

void mouseActiveMotion(int x, int y)
{
	// Spostamento su trackball del vettore posizione Camera 
	if (!moving_trackball) {
		return;
	}
	glm::vec3 destination = getTrackBallPoint(x, y);
	glm::vec3 origin = getTrackBallPoint(last_mouse_pos_X, last_mouse_pos_Y);
	float dx, dy, dz;
	dx = destination.x - origin.x;
	dy = destination.y - origin.y;
	dz = destination.z - origin.z;
	if (dx || dy || dz) {
		// rotation angle = acos( (v dot w) / (len(v) * len(w)) ) o approssimato da ||dest-orig||;
		float pi = glm::pi<float>();
		float angle = sqrt(dx * dx + dy * dy + dz * dz) * (180.0 / pi);
		// rotation axis = (dest vec orig) / (len(dest vec orig))
		glm::vec3 rotation_vec = glm::cross(origin, destination);
		// calcolo del vettore direzione w = C - A
		glm::vec4 direction = ViewSetup.position - ViewSetup.target;
		// rotazione del vettore direzione w 
		// determinazione della nuova posizione della camera 
		ViewSetup.position = ViewSetup.target + glm::rotate(glm::mat4(1.0f), glm::radians(-angle), rotation_vec) * direction;
	}
	last_mouse_pos_X = x; last_mouse_pos_Y = y;
	glutPostRedisplay();
}

void keyboardDown(unsigned char key, int x, int y)
{
	switch (key) {
		// Selezione della modalità di trasformazione
	case 'g':
		OperationMode = TRASLATING;
		break;
	case 'r':
		OperationMode = ROTATING;
		break;
	case 's':
		OperationMode = SCALING;
		break;
	case 27:
		glutLeaveMainLoop();
		break;
		// Selezione dell'asse
	case 'x':
		WorkingAxis = X;
		break;
	case 'y':
		WorkingAxis = Y;
		break;
	case 'z':
		WorkingAxis = Z;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		selected_obj = selected_obj > 0 ? selected_obj - 1 : objects.size() - 1;
		break;
	case GLUT_KEY_RIGHT:
		selected_obj = (selected_obj + 1) < objects.size() ? selected_obj + 1 : 0;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void main_menu_func(int option)
{
	switch (option)
	{
	case MenuOption::WIRE_FRAME: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case MenuOption::FACE_FILL: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case MenuOption::CULLING_ON: glEnable(GL_CULL_FACE);
		break;
	case MenuOption::CULLING_OFF: glDisable(GL_CULL_FACE);
		break;
	case MenuOption::CHANGE_TO_OCS: TransformMode = OCS;
		break;
	case MenuOption::CHANGE_TO_WCS: TransformMode = WCS;
		break;
	default:
		break;
	}
}

void material_menu_function(int option)
{
	objects[selected_obj].material = (MaterialType)option;
}

void buildOpenGLMenu()
{
	int materialSubMenu = glutCreateMenu(material_menu_function);

	glutAddMenuEntry(materials[MaterialType::RED_PLASTIC].name.c_str(), MaterialType::RED_PLASTIC);
	glutAddMenuEntry(materials[MaterialType::EMERALD].name.c_str(), MaterialType::EMERALD);
	glutAddMenuEntry(materials[MaterialType::BRASS].name.c_str(), MaterialType::BRASS);
	glutAddMenuEntry(materials[MaterialType::SLATE].name.c_str(), MaterialType::SLATE);
	glutAddMenuEntry(materials[MaterialType::SILVER].name.c_str(), MaterialType::SILVER);
	glutAddMenuEntry(materials[MaterialType::GOLD].name.c_str(), MaterialType::GOLD);
	glutAddMenuEntry(materials[MaterialType::CYAN_PLASTIC].name.c_str(), MaterialType::CYAN_PLASTIC);

	glutCreateMenu(main_menu_func); // richiama main_menu_func() alla selezione di una voce menu
	glutAddMenuEntry("Opzioni", -1); //-1 significa che non si vuole gestire questa riga
	glutAddMenuEntry("", -1);
	glutAddMenuEntry("Wireframe", MenuOption::WIRE_FRAME);
	glutAddMenuEntry("Face fill", MenuOption::FACE_FILL);
	glutAddMenuEntry("Culling: ON", MenuOption::CULLING_ON);
	glutAddMenuEntry("Culling: OFF", MenuOption::CULLING_OFF);
	glutAddSubMenu("Material", materialSubMenu);
	glutAddMenuEntry("World coordinate system", MenuOption::CHANGE_TO_WCS);
	glutAddMenuEntry("Object coordinate system", MenuOption::CHANGE_TO_OCS);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

glm::vec3 getTrackBallPoint(float x, float y)
{
	float zTemp;
	glm::vec3 point;
	//map to [-1;1]
	point.x = (2.0f * x - WindowWidth) / WindowWidth;
	point.y = (WindowHeight - 2.0f * y) / WindowHeight;

	zTemp = 1.0f - pow(point.x, 2.0) - pow(point.y, 2.0);
	point.z = (zTemp > 0.0f) ? sqrt(zTemp) : 0.0;
	return point;
}

void moveCameraForeward()
{
	glm::vec4 direction = ViewSetup.target - ViewSetup.position;
	ViewSetup.position -= direction * CAMERA_ZOOM_SPEED;
}

void moveCameraBack()
{
	glm::vec4 direction = ViewSetup.target - ViewSetup.position;
	ViewSetup.position += direction * CAMERA_ZOOM_SPEED;
}

void moveCameraLeft()
{
	//TODO
	glm::vec3 direction = ViewSetup.target - ViewSetup.position;
	glm::vec3 slide = glm::cross(direction, glm::vec3(ViewSetup.upVector)) * CAMERA_TRASLATION_SPEED;
	ViewSetup.position += glm::vec4(slide, 0.0);
	ViewSetup.target += glm::vec4(slide, 0.0);
}

void moveCameraRight()
{
	//TODO
	glm::vec3 direction = ViewSetup.target - ViewSetup.position;
	glm::vec3 slide = glm::cross(direction, glm::vec3(ViewSetup.upVector)) * CAMERA_TRASLATION_SPEED;
	ViewSetup.position -= glm::vec4(slide, 0.0);
	ViewSetup.target -= glm::vec4(slide, 0.0);
}

void moveCameraUp()
{
	glm::vec3 direction = ViewSetup.target - ViewSetup.position;
	glm::vec3 slide_vector = glm::normalize(glm::cross(direction, glm::vec3(ViewSetup.upVector)));
	glm::vec3 upDirection = glm::cross(direction, slide_vector) * CAMERA_TRASLATION_SPEED;
	ViewSetup.position -= glm::vec4(upDirection, 0.0);
	ViewSetup.target -= glm::vec4(upDirection, 0.0);
}

void moveCameraDown()
{
	glm::vec4 direction = ViewSetup.target - ViewSetup.position;
	glm::vec3 slide_vector = glm::normalize(glm::cross(glm::vec3(direction), glm::vec3(ViewSetup.upVector)));
	glm::vec3 upDirection = glm::cross(glm::vec3(direction), slide_vector) * CAMERA_TRASLATION_SPEED;
	ViewSetup.position += glm::vec4(upDirection, 0.0);
	ViewSetup.target += glm::vec4(upDirection, 0.0);
}

void modifyModelMatrix(glm::vec3 translation_vector, glm::vec3 rotation_vector, GLfloat angle, GLfloat scale_factor) {
	glm::mat4 M = objects[selected_obj].M;

	// TRANSLATION
	if(translation_vector.length() != 0) {
		M = glm::translate(M, translation_vector);
	}

	// ROTATION
	if(angle != 0) {
		if(TransformMode == OCS) {
			M = glm::rotate(M, glm::radians(angle), rotation_vector);
		} else if(TransformMode == WCS) {
			const glm::vec3 position = M[3];
			const glm::vec3 origin = Axis.M[3];

			M = glm::translate(M, origin - position);
			M = glm::rotate(M, glm::radians(angle), rotation_vector);
			M = glm::translate(M, position - origin);
		}
	}

	// SCALE
	if(scale_factor != 1.0) {
		M = glm::scale(M, glm::vec3(scale_factor));
	}

	objects[selected_obj].M = M;

}

void generate_and_load_buffers(bool generate, Mesh* mesh)
{
	if (generate) {
		// Genero 1 Vertex Array Object
		glGenVertexArrays(1, &mesh->vertexArrayObjID);
		// Genero 1 Vertex Buffer Object per i vertici
		glGenBuffers(1, &mesh->vertexBufferObjID);
		// Genero 1 Buffer Object per le normali
		glGenBuffers(1, &mesh->normalBufferObjID);
	}

	glBindVertexArray(mesh->vertexArrayObjID);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(glm::vec3), &mesh->vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,					// attribute index in the shader
		3,                  // size
		GL_FLOAT,           // type
		false,              // normalized 
		0,					// stride
		(void*)0            // array buffer offset
	);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->normalBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, mesh->normals.size() * sizeof(glm::vec3), mesh->normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,					// attribute index in the shader
		3,                  // size
		GL_FLOAT,           // type
		false,              // normalized 
		0,					// stride
		(void*)0            // array buffer offset
	);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void loadObjFile(string file_path, Mesh* mesh)
{
	FILE* file = fopen(file_path.c_str(), "r");
	if (file == NULL) {
		std::cerr << "\nFailed to open obj file! --> " << file_path << std::endl;
		std::getchar();
		exit(EXIT_FAILURE);
	}
	// tmp data structures
	vector<GLuint> vertexIndices, normalIndices, uvIndices;
	vector<glm::vec3> tmp_vertices, tmp_normals;
	vector<glm::vec2> tmp_uvs;

	char lineHeader[128];
	while (fscanf(file, "%s", lineHeader) != EOF) {
		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, " %f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			tmp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, " %f %f %f\n", &normal.x, &normal.y, &normal.z);
			tmp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, " %f %f\n", &uv.x, &uv.y);
			uv.y = 1 - uv.y;
			tmp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			GLuint v_a, v_b, v_c; // index in position array
			GLuint n_a, n_b, n_c; // index in normal array
			GLuint t_a, t_b, t_c; // index in UV array

			fscanf(file, "%s", lineHeader);
			if (strstr(lineHeader, "//")) { // case: v//n v//n v//n
				sscanf(lineHeader, "%d//%d", &v_a, &n_a);
				fscanf(file, "%d//%d %d//%d\n", &v_b, &n_b, &v_c, &n_c);
				n_a--, n_b--, n_c--;
				normalIndices.push_back(n_a); normalIndices.push_back(n_b); normalIndices.push_back(n_c);
			}
			else if (strstr(lineHeader, "/")) {// case: v/t/n v/t/n v/t/n
				sscanf(lineHeader, "%d/%d/%d", &v_a, &t_a, &n_a);
				fscanf(file, "%d/%d/%d %d/%d/%d\n", &v_b, &t_b, &n_b, &v_c, &t_c, &n_c);
				n_a--, n_b--, n_c--;
				t_a--, t_b--, t_c--;
				normalIndices.push_back(n_a); normalIndices.push_back(n_b); normalIndices.push_back(n_c);
				uvIndices.push_back(t_a); uvIndices.push_back(t_b); uvIndices.push_back(t_c);
			}
			else {// case: v v v
				sscanf(lineHeader, "%d", &v_a);
				fscanf(file, "%d %d\n", &v_b, &v_c);
			}
			v_a--; v_b--; v_c--;
			vertexIndices.push_back(v_a); vertexIndices.push_back(v_b); vertexIndices.push_back(v_c);
		}
	}
	fclose(file);

	// If normals and uvs are not loaded, we calculate normals for vertex
	if (tmp_normals.size() == 0) {
		tmp_normals.resize(vertexIndices.size() / 3, glm::vec3(0.0, 0.0, 0.0));
		// normal of each face saved 1 time PER FACE!
		for (int i = 0; i < vertexIndices.size(); i += 3)
		{
			GLushort ia = vertexIndices[i];
			GLushort ib = vertexIndices[i + 1];
			GLushort ic = vertexIndices[i + 2];
			glm::vec3 normal = glm::normalize(glm::cross(
				glm::vec3(tmp_vertices[ib]) - glm::vec3(tmp_vertices[ia]),
				glm::vec3(tmp_vertices[ic]) - glm::vec3(tmp_vertices[ia])));
			
			//Normali ai vertici
			tmp_normals[ia] += normal;
			tmp_normals[ib] += normal;
			tmp_normals[ic] += normal;
			//Put an index to the normal for all 3 vertex of the face
			normalIndices.push_back(ia);
			normalIndices.push_back(ib);
			normalIndices.push_back(ic);

			// Normali alle facce
			//tmp_normals[i / 3] = normal;
			//Put an index to the normal for all 3 vertex of the face
			//normalIndices.push_back(i / 3);
			//normalIndices.push_back(i / 3);
			//normalIndices.push_back(i / 3);
		}
	}

	// We prepare the data for glDrawArrays calls, this is a simple but non optimal way of storing mesh data.
	// However, you could optimize the mesh data using a index array for both vertex positions, 
	// normals and textures and later use glDrawElements
	int i = 0;
	// Now following the index arrays, we build the final arrays that will contain the data for glDrawArray...
	for (int i = 0; i < vertexIndices.size(); i++) {
		mesh->vertices.push_back(tmp_vertices[vertexIndices[i]]);
		//mesh->normals.push_back(tmp_normals[normalIndices[i]]);
		mesh->normals.push_back(glm::normalize(tmp_normals[normalIndices[i]]));
	}
}
void drawAxisAndGrid()
{
	glUseProgram(shaders_IDs[Grid.shading]);
	glUniformMatrix4fv(base_uniforms[Grid.shading].M_Matrix_pointer, 1, GL_FALSE, value_ptr(Grid.M));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(Grid.mesh.vertexArrayObjID);
	glDrawArrays(GL_TRIANGLES, 0, Grid.mesh.vertices.size());
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	
	glUseProgram(shaders_IDs[Axis.shading]);
	glUniformMatrix4fv(base_uniforms[Axis.shading].M_Matrix_pointer, 1, GL_FALSE, value_ptr(Axis.M));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(Axis.mesh.vertexArrayObjID);
	glDrawArrays(GL_TRIANGLES, 0, Axis.mesh.vertices.size());
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void printToScreen()
{
	string axis = "Axis: ";
	string mode = "Navigate/Modify: ";
	string obj = "Object: " + objects[selected_obj].name;
	string ref = "WCS/OCS: ";
	string mat = "Material: " + materials[objects[selected_obj].material].name;
	switch (WorkingAxis) {
	case X: axis += "X"; break;
	case Y: axis += "Y"; break;
	case Z: axis += "Z"; break;
	}
	switch (OperationMode) {
	case TRASLATING: mode += "Translate"; break;
	case ROTATING: mode += "Rotate"; break;
	case SCALING: mode += "Scale"; break;
	case NAVIGATION: mode += "Navigate"; break;
	}
	switch (TransformMode) {
	case OCS: ref += "OCS"; break;
	case WCS: ref += "WCS"; break;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Handle output info layout 
	gluOrtho2D(0, WindowHeight * aspect_ratio, 0, WindowHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vector<string> lines;
	lines.push_back(mat);
	lines.push_back(obj);
	lines.push_back(axis);
	lines.push_back(mode);
	lines.push_back(ref);
	glDisable(GL_DEPTH_TEST);
	HUD_Logger::get()->printInfo(lines);
	glEnable(GL_DEPTH_TEST);

	resize(WindowWidth, WindowHeight);
}


int main(int argc, char** argv) {

	GLboolean GlewInitResult;
	glutInit(&argc, argv);
	glutSetOption(GLUT_MULTISAMPLE, 4);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Model Viewer ");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboardDown);
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseActiveMotion);
	glutSpecialFunc(special);

	glewExperimental = GL_TRUE;
	GlewInitResult = glewInit();
	fprintf(
		stdout,
		"INFO: OpenGL Version: %s\n",
		glGetString(GL_VERSION)
	);

	init();
	initShader();

	buildOpenGLMenu();
	refresh_monitor(16);
	glutMainLoop();
}

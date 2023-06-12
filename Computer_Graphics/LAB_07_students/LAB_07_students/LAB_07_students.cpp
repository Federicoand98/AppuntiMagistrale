/******************************************************************************************
LAB 07
Gestione di texture/env/normal mapping su oggetti di una scena 3D.
Interazione mediante controllo da mouse e tastiera.
I modelli geometrici in scena sono mesh poligonali in formato *.obj
ed un oggetto parametrico (torus)

CTRL+WHEEL	= pan orizzontale della telecamera
SHIFT+WHEEL = pan verticale della telecamera
WHEEL		= se navigazione  --> ZOOM IN/OUT
			  se modify       --> agisce sulla trasformazione dell'oggetto
g r s		per le modalità di lavoro: traslate/rotate/scale
x y z		per l'asse di lavoro
wcs/ocs		selezionabili dal menu pop-up

OpenGL Mathematics (GLM) is a header only C++ mathematics library for graphics software
based on the OpenGL Shading Language (GLSL) specifications.
*******************************************************************************************/

#define _CRT_SECURE_NO_WARNINGS // for fscanf

#include <stdio.h>
#include <math.h>
#include <string>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "HUD_Logger.h"
#include "ShaderMaker.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SHIFT_WHEEL_UP 11
#define SHIFT_WHEEL_DOWN 12
#define CTRL_WHEEL_UP 19
#define CTRL_WHEEL_DOWN 20

#define NUM_SHADERS 7
#define NUM_LIGHT_SHADERS 2

using namespace std;

// Viewport size
static int WindowWidth = 1120;
static int WindowHeight = 630;
GLfloat aspect_ratio = 16.0f / 9.0f;

typedef struct {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> tangents;
	GLuint vertexArrayObjID;
	GLuint vertexBufferObjID;
	GLuint normalBufferObjID;
	GLuint uvBufferObjID;
	GLuint tgBufferObjID;
} Mesh;

typedef enum {
	RED_PLASTIC,
	EMERALD,
	BRASS,
	SLATE,
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
	NORMAL_MAPPING,
	TEXTURE_PHONG,
	SKYBOX,
	REFLECTION,
	REFRACTION,
	TEXTURE_ONLY,
	PASS_THROUGH
} ShadingType;

typedef struct {
	Mesh mesh;
	MaterialType material;
	ShadingType shading;
	GLuint diffuseTexID;
	GLuint normalTexID;
	glm::mat4 M;
	string name;
	bool blended;
} Object;

typedef struct {
	GLuint light_position_pointer;
	GLuint light_color_pointer;
	GLuint light_power_pointer;
	GLuint material_diffuse;
	GLuint material_ambient;
	GLuint material_specular;
	GLuint material_shininess;
	GLuint diffuse_sampler;
	GLuint normal_sampler;
} LightShaderUniform;

typedef struct {
	GLuint P_Matrix_pointer;
	GLuint V_Matrix_pointer;
	GLuint M_Matrix_pointer;
	GLuint camera_position_pointer;
} BaseShaderUniform;

const string MeshDir = "Mesh/";
const string TextureDir = "Textures/";
const string ShaderDir = "Shaders/";
static GLuint cubeTexture;
static vector<int> movables; // Objects that the user can move an focus on
static vector<int> transparents; // Objects to be drawn later for blending reasons
static vector<Object> objects; // All 3D stuff
static vector<Material> materials;
static int selected_obj = 0;

struct {
	// Variables controlling the torus mesh resolution
	int NumWraps = 10;
	int NumPerWrap = 8;
	// Variables controlling the size of the torus
	float MajorRadius = 3.0;
	float MinorRadius = 1.0;
	// Variables controlling the texture distribution
	int TextureWrapVert = 1;
	int TextureWrapHoriz = 1;
	int torus_index;
} TorusSetup;

// Materiali disponibili
glm::vec3 red_plastic_ambient = { 0.1, 0.0, 0.0 }, red_plastic_diffuse = { 0.6, 0.1, 0.1 }, red_plastic_specular = { 0.6, 0.6, 0.6 }; GLfloat red_plastic_shininess = 32.0f;
glm::vec3 brass_ambient = { 0.1, 0.06, 0.015 }, brass_diffuse = { 0.78, 0.57, 0.11 }, brass_specular = { 0.99, 0.91, 0.91 }; GLfloat brass_shininess = 27.8f;
glm::vec3 emerald_ambient = { 0.0215, 0.04745, 0.0215 }, emerald_diffuse = { 0.07568, 0.71424, 0.07568 }, emerald_specular = { 0.633,0.633, 0.633 }; GLfloat emerald_shininess = 78.8f;
glm::vec3 slate_ambient = { 0.02, 0.02, 0.02 }, slate_diffuse = { 0.1, 0.1, 0.1 }, slate_specular{ 0.3, 0.3, 0.3 }; GLfloat slate_shininess = 20.78125f;

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
void initShader();
void init();
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
//	Crea ed applica la matrice di trasformazione alla matrice dell'oggeto discriminando tra WCS e OCS.
//	La funzione è gia invocata con un input corretto, è sufficiente concludere la sua implementazione.
void modifyModelMatrix(glm::vec3 translation_vector, glm::vec3 rotation_vector, GLfloat angle, GLfloat scale_factor);
/* Mesh Functions*/
void compute_Torus(Mesh* mesh);
// Genera i buffer per la mesh in input e ne salva i puntatori di openGL
void generate_and_load_buffers(bool generate, Mesh* mesh);
// legge un file obj ed inizializza i vector della mesh in input
void loadObjFile(string file_path, Mesh* mesh);
//Uses stb_image.h to read an image, then loads it.
GLuint loadTexture(string path);
//Loads 6 images to build a cube map
GLuint load_cube_map_texture(string face_textures[6]);
//Surface Tangents calculations based on edges distances and uv distances, Bitangents are calculated in the vertex shader
void calc_tangents(Mesh* mesh);
// 2D fixed pipeline Font rendering on screen
void printToScreen();

void init_light_object() {
	Mesh sphereS = {};
	loadObjFile(MeshDir + "sphere_n_t_smooth.obj", &sphereS);
	generate_and_load_buffers(true, &sphereS);
	Object obj = {};
	obj.mesh = sphereS;
	obj.material = MaterialType::NO_MATERIAL;
	obj.shading = ShadingType::PASS_THROUGH;
	obj.name = "light";
	obj.M = glm::scale(glm::translate(glm::mat4(1), light.position), glm::vec3(0.2, 0.2, 0.2));
	objects.push_back(obj);
	movables.push_back(objects.size() - 1);
}

void init_reflective_sphere() {
	Mesh surface = {};
	loadObjFile(MeshDir + "sphere_n_t_smooth.obj", &surface);
	calc_tangents(&surface);
	generate_and_load_buffers(true, &surface);
	Object obj = {};
	obj.mesh = surface;
	obj.diffuseTexID = cubeTexture;
	obj.material = MaterialType::NO_MATERIAL;
	obj.shading = ShadingType::TEXTURE_ONLY;// PASS_THROUGH; //TEXTURE_ONLY; //  TODO REFLECTION;
	obj.name = "Mirror Ball";
	obj.M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-3, 2, -6)), glm::vec3(1.0, 1.0, 1.0));
	objects.push_back(obj);
	movables.push_back(objects.size() - 1);
}

void init_refractive_obj() {
	Mesh surface = {};
	loadObjFile(MeshDir + "cube_n_t_flat.obj", &surface);
	calc_tangents(&surface);
	generate_and_load_buffers(true, &surface);
	Object obj = {};
	obj.mesh = surface;
	obj.diffuseTexID = cubeTexture;
	obj.material = MaterialType::NO_MATERIAL;
	obj.shading = ShadingType::TEXTURE_ONLY; // PASS_THROUGH;  // TODO REFRACTION
	obj.name = "Glass Cube";
	obj.M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-6, 2, -3)), glm::vec3(2.0, 2.0, 2.0));
	objects.push_back(obj);
	movables.push_back(objects.size() - 1);
}
//Textured plane (2 triangles) with a procedural texture but no material, use a texture-only shader
void init_textured_plane() {

	GLuint textureID;

	Mesh surface = {};
	surface.vertices = { {-2,0,-2}, {-2,0,2}, {2,0,2}, {2,0,-2}, {-2,0,-2}, {2,0,2} };
	surface.normals = { {0,1,0}, {0,1,0}, {0,1,0}, {0,1,0}, {0,1,0}, {0,1,0} };
	//Tex coords are out of bound to show the GL_REPEAT effect
	//surface.texCoords = { {-4,-4}, {-4,+4}, {4,4}, {4,-4}, {-4,-4}, {4,4} }; // out of bound UVs
	surface.texCoords = { {0,0}, {0,1}, {1,1}, {1,0}, {0,0}, {1,1} }; // standard UVs
	generate_and_load_buffers(true, &surface);
	Object obj0 = {};
	obj0.mesh = surface;
	obj0.material = MaterialType::NO_MATERIAL;
	obj0.shading = ShadingType::TEXTURE_ONLY;

	/////////////////////////////////////////////////////////////////////////
	//  Compute checkboard procedural_texture image of dimension width x width x 3 (RGB)
	/////////////////////////////////////////////////////////////////////////
	GLubyte image[64][64][3];
	int i, j, c;
	for (i = 0; i < 64; i++) {
		for (j = 0; j < 64; j++) {
			c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
			image[i][j][0] = (GLubyte)c;
			image[i][j][1] = (GLubyte)c;
			image[i][j][2] = (GLubyte)c;
		}
	}
	/////////////////////////////////////////
	glGenTextures(1, &obj0.diffuseTexID);
	glBindTexture(GL_TEXTURE_2D, obj0.diffuseTexID);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,  //the target
		0, // the mip map level we want to generate
		GL_RGB, // the format of the texture
		64, //texture_size, width
		64, //texture_size, heigth
		0,  // border, leave 0
		GL_RGB, // we assume is a RGB color image with 24 bit depth per pixel
		GL_UNSIGNED_BYTE, // the data type
		image);
	obj0.name = "Plane";
	obj0.M = glm::translate(glm::mat4(1), glm::vec3(0., 0., -5.));
	objects.push_back(obj0);
}

void init_rock() {
	Mesh surface = {};
	loadObjFile(MeshDir + "sharprockfree.obj", &surface);
	calc_tangents(&surface);
	generate_and_load_buffers(true, &surface);
	Object obj = {};
	obj.mesh = surface;
	obj.diffuseTexID = loadTexture(TextureDir + "sharprockfree_default_color.png");
	obj.normalTexID = loadTexture(TextureDir + "sharprockfree_default_nmap-dx.png");
	obj.material = MaterialType::SLATE;
	obj.shading = ShadingType::NORMAL_MAPPING; //TEXTURE_ONLY; // TODO NORMAL_MAPPING;
	obj.name = "Sharpy Rock";
	obj.M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(5, 0.75, 5)), glm::vec3(0.02, 0.02, 0.02));
	objects.push_back(obj);
	movables.push_back(objects.size() - 1);
}

void init_brick_column() {
	Mesh surface = {};
	loadObjFile(MeshDir + "column.obj", &surface);
	calc_tangents(&surface);
	generate_and_load_buffers(true, &surface);
	Object obj = {};
	obj.mesh = surface;
	obj.diffuseTexID = loadTexture(TextureDir + "brickwall.jpg");
	obj.normalTexID = loadTexture(TextureDir + "brickwall_normal.jpg");
	obj.material = MaterialType::SLATE;
	obj.shading = ShadingType::NORMAL_MAPPING; //TEXTURE_ONLY; // TODO NORMAL_MAPPING;
	obj.name = "Brick Wall Normal Mapping";
	obj.M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(6, 2, -3)), glm::vec3(4., 4., 4.));
	objects.push_back(obj);
	movables.push_back(objects.size() - 1);
	obj.shading = ShadingType::TEXTURE_ONLY; // TEXTURE_PHONG; // TODO NORMAL_MAPPING;
	obj.name = "Brick Wall Texture Mapping";
	obj.M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(3, 2, -6)), glm::vec3(4., 4., 4.));
	objects.push_back(obj);
	movables.push_back(objects.size() - 1);
}

void init_skybox() {
	Mesh surface = {};
	loadObjFile(MeshDir + "cube_n_t_flat.obj", &surface);
	std::reverse(surface.vertices.begin(), surface.vertices.end()); // reverse draw order to counter ClockWise face culling
	generate_and_load_buffers(true, &surface);
	Object obj = {};
	obj.mesh = surface;
	obj.diffuseTexID = cubeTexture;
	obj.material = MaterialType::NO_MATERIAL;
	obj.shading = ShadingType::SKYBOX;
	obj.name = "skybox";
	//  TODO posizionamento cubo contenente la scena
	obj.M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(5, 5, 5)), glm::vec3(1., 1., 1.));
	objects.push_back(obj);
}

void init_windows() {
	Mesh surface = {};
	loadObjFile(MeshDir + "window.obj", &surface);
	generate_and_load_buffers(true, &surface);
	Object obj = {};
	obj.mesh = surface;
	obj.diffuseTexID = loadTexture(TextureDir + "winTex2.png");;
	obj.material = MaterialType::NO_MATERIAL;
	obj.shading = ShadingType::TEXTURE_ONLY;
	obj.name = "Little Window";
	obj.M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0, 0, -10)), glm::vec3(0.5, 0.5, 0.2));
	obj.blended = true; // TODO false -> object solid; true -> object semi-transparent if Blending enabled;
	objects.push_back(obj);
	movables.push_back(objects.size() - 1);
	transparents.push_back(objects.size() - 1);
	obj.name = "Big Window";
	obj.M = glm::scale(glm::translate(glm::mat4(1), glm::vec3(0, 0, -8)), glm::vec3(1., 1., 0.2));
	objects.push_back(obj);
	movables.push_back(objects.size() - 1);
	transparents.push_back(objects.size() - 1);
}

void init_torus() {
	Mesh torus = {};
	compute_Torus(&torus);
	generate_and_load_buffers(true, &torus);
	Object obj = {};
	obj.mesh = torus;
	obj.material = MaterialType::BRASS; // NO_MATERIAL;
	obj.shading = ShadingType::PASS_THROUGH; //TEXTURE_ONLY;  // PASS_THROUGH; // TEXTURE_PHONG;  
	obj.name = "Torus";
	obj.diffuseTexID = loadTexture(TextureDir + "WoodGrain.bmp");
	//obj.diffuseTexID = loadTexture(TextureDir + "brickwall.jpg");
	obj.M = glm::translate(glm::mat4(1), glm::vec3(-5., 0., 5.));
	objects.push_back(obj);
	movables.push_back(objects.size() - 1);
	TorusSetup.torus_index = objects.size() - 1;
}

void init_axis() {
	Mesh _grid = {};
	loadObjFile(MeshDir + "axis.obj", &_grid);
	generate_and_load_buffers(true, &_grid);
	Object obj = {};
	obj.mesh = _grid;
	obj.material = MaterialType::NO_MATERIAL;
	obj.shading = ShadingType::TEXTURE_ONLY;
	obj.diffuseTexID = loadTexture(TextureDir + "AXIS_TEX.png");
	obj.name = "axis";
	obj.M = glm::scale(glm::mat4(1), glm::vec3(2.f, 2.f, 2.f));
	objects.push_back(obj);
}

void init_grid() {
	Mesh _grid = {};
	loadObjFile(MeshDir + "reference_grid.obj", &_grid);
	generate_and_load_buffers(true, &_grid);
	Object obj = {};
	obj.mesh = _grid;
	obj.material = MaterialType::NO_MATERIAL;
	obj.shading = ShadingType::PASS_THROUGH;
	obj.name = "grid_";
	obj.M = glm::scale(glm::mat4(1), glm::vec3(2.f, 2.f, 2.f));
	objects.push_back(obj);
}

void init() {
	// Default render settings
	OperationMode = NAVIGATION;
	glEnable(GL_DEPTH_TEST);	// Hidden surface removal
	glCullFace(GL_BACK);	// remove faces facing the background
	glEnable(GL_LINE_SMOOTH);
	// Blending set up
	//TODO enable alpha blending for the windows  --->	
	glEnable(GL_BLEND);
	// The blending function tells the pipeline how to mix the color of a transparent object with the background.
	// The factor of the source color is the source color alpha,
	// The factor of the destination color is calculated as 1 - alpha of the source color
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Camera Setup
	ViewSetup = {};
	ViewSetup.position = glm::vec4(10.0, 10.0, 10.0, 0.0);
	ViewSetup.target = glm::vec4(0.0, 0.0, 0.0, 0.0);
	ViewSetup.upVector = glm::vec4(0.0, 1.0, 0.0, 0.0);
	PerspectiveSetup = {};
	PerspectiveSetup.aspect = (GLfloat)WindowWidth / (GLfloat)WindowHeight;
	PerspectiveSetup.fovY = 45.0f;
	PerspectiveSetup.far_plane = 2000.0f;
	PerspectiveSetup.near_plane = 0.1f;

	//Light initialization
	light.position = { 0.0,5.0,0.0 };
	light.color = { 1.0,1.0,1.0 };
	light.power = 2.f;

	// Materials setup
	materials.resize(5);
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

	materials[MaterialType::NO_MATERIAL].name = "NO_MATERIAL";
	materials[MaterialType::NO_MATERIAL].ambient = glm::vec3(1, 1, 1);
	materials[MaterialType::NO_MATERIAL].diffuse = glm::vec3(0, 0, 0);
	materials[MaterialType::NO_MATERIAL].specular = glm::vec3(0, 0, 0);
	materials[MaterialType::NO_MATERIAL].shininess = 1.f;

}


void initShader()
{
	// SHADERS configuration section
	shaders_IDs.resize(NUM_SHADERS);
	light_uniforms.resize(NUM_LIGHT_SHADERS); // allocate space for uniforms with light
	base_uniforms.resize(NUM_SHADERS); // allocate space for uniforms of all shaders

	//NORMAL_MAPPING Shader loading
	shaders_IDs[NORMAL_MAPPING] = createProgram(ShaderDir + "v_normal_map.glsl", ShaderDir + "f_normal_map.glsl");
	BaseShaderUniform base_unif = {};
	//Otteniamo i puntatori alle variabili uniform per poterle utilizzare in seguito
	base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "M");
	base_unif.camera_position_pointer = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "camera_position");
	base_uniforms[ShadingType::NORMAL_MAPPING] = base_unif;
	LightShaderUniform light_unif = {};
	light_unif.material_ambient = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "material.ambient");
	light_unif.material_diffuse = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "material.diffuse");
	light_unif.material_specular = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "material.specular");
	light_unif.material_shininess = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "material.shininess");
	light_unif.light_position_pointer = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "light.position");
	light_unif.light_color_pointer = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "light.color");
	light_unif.light_power_pointer = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "light.power");
	light_unif.diffuse_sampler = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "diffuseMap");
	light_unif.normal_sampler = glGetUniformLocation(shaders_IDs[NORMAL_MAPPING], "normalMap");
	light_uniforms[ShadingType::NORMAL_MAPPING] = light_unif;
	//Rendiamo attivo lo shader
	glUseProgram(shaders_IDs[NORMAL_MAPPING]);
	//Shader uniforms initialization
	glUniform3f(light_uniforms[NORMAL_MAPPING].light_position_pointer, light.position.x, light.position.y, light.position.z);
	glUniform3f(light_uniforms[NORMAL_MAPPING].light_color_pointer, light.color.r, light.color.g, light.color.b);
	glUniform1f(light_uniforms[NORMAL_MAPPING].light_power_pointer, light.power);
	glUniform1i(light_uniforms[NORMAL_MAPPING].diffuse_sampler, 0); // index for GL_TEXTURE0
	glUniform1i(light_uniforms[NORMAL_MAPPING].normal_sampler, 1); // index for GL_TEXTURE1

	//TEXTURE_PHONG Shader loading
	//shaders_IDs[TEXTURE_PHONG] = createProgram(ShaderDir + "v_texture_phong.glsl", ShaderDir + "f_texture_phong.glsl");
	//Otteniamo i puntatori alle variabili uniform per poterle utilizzare in seguito
	/*base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "M");
	base_unif.camera_position_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "camera_position");
	base_uniforms[ShadingType::TEXTURE_PHONG] = base_unif;
	light_unif.material_ambient = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "material.ambient");
	light_unif.material_diffuse = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "material.diffuse");
	light_unif.material_specular = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "material.specular");
	light_unif.material_shininess = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "material.shininess");
	light_unif.light_position_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "light.position");
	light_unif.light_color_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "light.color");
	light_unif.light_power_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "light.power");
	light_unif.diffuse_sampler = glGetUniformLocation(shaders_IDs[TEXTURE_PHONG], "diffuseMap");
	light_uniforms[ShadingType::TEXTURE_PHONG] = light_unif;
	//Rendiamo attivo lo shader
	glUseProgram(shaders_IDs[TEXTURE_PHONG]);
	//Shader uniforms initialization
	glUniform3f(light_uniforms[TEXTURE_PHONG].light_position_pointer, light.position.x, light.position.y, light.position.z);
	glUniform3f(light_uniforms[TEXTURE_PHONG].light_color_pointer, light.color.r, light.color.g, light.color.b);
	glUniform1f(light_uniforms[TEXTURE_PHONG].light_power_pointer, light.power);
	glUniform1i(light_uniforms[TEXTURE_PHONG].diffuse_sampler, 0); // index for GL_TEXTURE0
	*/

	//SKYBOX Shader loading
	shaders_IDs[SKYBOX] = createProgram(ShaderDir + "v_skybox_advanced.glsl", ShaderDir + "f_skybox.glsl");
	//Otteniamo i puntatori alle variabili uniform per poterle utilizzare in seguito
	base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[SKYBOX], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[SKYBOX], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[SKYBOX], "M");
	base_unif.camera_position_pointer = glGetUniformLocation(shaders_IDs[SKYBOX], "camera_position");
	//Rendiamo attivo lo shader
	glUseProgram(shaders_IDs[SKYBOX]);
	glUniform1i(glGetUniformLocation(shaders_IDs[SKYBOX], "skybox"), 0);
	base_uniforms[ShadingType::SKYBOX] = base_unif;

	//REFLECTION Shader loading
	// TODO REFLECTION
	
	//REFRACTION Shader loading
	// TODO REFRACTION

	//Texture Shader loading
	shaders_IDs[TEXTURE_ONLY] = createProgram(ShaderDir + "v_texture.glsl", ShaderDir + "f_texture.glsl");
	//Otteniamo i puntatori alle variabili uniform per poterle utilizzare in seguito
	base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_ONLY], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_ONLY], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_ONLY], "M");
	base_unif.camera_position_pointer = glGetUniformLocation(shaders_IDs[TEXTURE_ONLY], "camera_position");
	base_uniforms[ShadingType::TEXTURE_ONLY] = base_unif;

	//PASS-THROUGH Shader loading
	shaders_IDs[PASS_THROUGH] = createProgram(ShaderDir + "v_passthrough.glsl", ShaderDir + "f_passthrough.glsl");
	//Otteniamo i puntatori alle variabili uniform per poterle utilizzare in seguito
	base_unif.P_Matrix_pointer = glGetUniformLocation(shaders_IDs[PASS_THROUGH], "P");
	base_unif.V_Matrix_pointer = glGetUniformLocation(shaders_IDs[PASS_THROUGH], "V");
	base_unif.M_Matrix_pointer = glGetUniformLocation(shaders_IDs[PASS_THROUGH], "M");
	base_unif.camera_position_pointer = glGetUniformLocation(shaders_IDs[PASS_THROUGH], "camera_position");
	base_uniforms[ShadingType::PASS_THROUGH] = base_unif;
	//Rendiamo attivo lo shader
	glUseProgram(shaders_IDs[PASS_THROUGH]);
	glUniform4fv(glGetUniformLocation(shaders_IDs[PASS_THROUGH], "Color"), 1, value_ptr(glm::vec4(1.0, 1.0, 1.0, 1.0)));

	// Cube Texture
	string skybox_textures[6] = {
	TextureDir + "cape_cubemap/" + "px.png",//GL_TEXTURE_CUBE_MAP_POSITIVE_X 	Right
	TextureDir + "cape_cubemap/" + "nx.png",//GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	Left
	TextureDir + "cape_cubemap/" + "py.png",//GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	Top
	TextureDir + "cape_cubemap/" + "ny.png",//GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	Bottom
	TextureDir + "cape_cubemap/" + "pz.png",//GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	Back
	TextureDir + "cape_cubemap/" + "nz.png" //GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	Front
	};
	//string skybox_textures[6] = {
	//TextureDir + "cape_cubemap/" + "posx3.bmp",//GL_TEXTURE_CUBE_MAP_POSITIVE_X 	Right
	//TextureDir + "cape_cubemap/" + "negx3.bmp",//GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	Left
	//TextureDir + "cape_cubemap/" + "posy3.bmp",//GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	Top
	//TextureDir + "cape_cubemap/" + "negy3.bmp",//GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	Bottom
	//TextureDir + "cape_cubemap/" + "posz3.bmp",//GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	Back
	//TextureDir + "cape_cubemap/" + "negz3.bmp" //GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	Front
	//};
	cubeTexture = load_cube_map_texture(skybox_textures);

	//////////////////////////////////////////////////////////////////////
	//				OBJECTS IN SCENE
	//////////////////////////////////////////////////////////////////////

	//Skybox should be first, see the display function...
	init_skybox();

	//PLANE with a procedural texture but no material, uses a texture-only shader
	init_textured_plane();

	// a cool rock  with normal mapping
	init_rock();

	//Brick wall with normal mapping
	init_brick_column();

	//Full reflective sphere
	init_reflective_sphere();

	//Full refractive sphere
	init_refractive_obj();

	//Reference point of the position of the light
	init_light_object();

	// White Axis
	init_axis();

	// White Grid for reference
	init_grid();

	// white window
	init_windows();

	//The torus
	init_torus();


}

void drawScene() {
	glClearColor(0.4, 0.4, 0.4, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < objects.size(); i++) {
		// TODO if you handle transparency then you must jump any semi transparent object

		//Shader selection
		switch (objects[i].shading) {
		case ShadingType::NORMAL_MAPPING:
			glUseProgram(shaders_IDs[NORMAL_MAPPING]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[NORMAL_MAPPING].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			//Material loading
			glUniform3fv(light_uniforms[NORMAL_MAPPING].material_ambient, 1, glm::value_ptr(materials[objects[i].material].ambient));
			glUniform3fv(light_uniforms[NORMAL_MAPPING].material_diffuse, 1, glm::value_ptr(materials[objects[i].material].diffuse));
			glUniform3fv(light_uniforms[NORMAL_MAPPING].material_specular, 1, glm::value_ptr(materials[objects[i].material].specular));
			glUniform1f(light_uniforms[NORMAL_MAPPING].material_shininess, materials[objects[i].material].shininess);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, objects[i].diffuseTexID);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, objects[i].normalTexID);
			break;
		case ShadingType::TEXTURE_PHONG:
			glUseProgram(shaders_IDs[TEXTURE_PHONG]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[TEXTURE_PHONG].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			//Material loading
			glUniform3fv(light_uniforms[TEXTURE_PHONG].material_ambient, 1, glm::value_ptr(materials[objects[i].material].ambient));
			glUniform3fv(light_uniforms[TEXTURE_PHONG].material_diffuse, 1, glm::value_ptr(materials[objects[i].material].diffuse));
			glUniform3fv(light_uniforms[TEXTURE_PHONG].material_specular, 1, glm::value_ptr(materials[objects[i].material].specular));
			glUniform1f(light_uniforms[TEXTURE_PHONG].material_shininess, materials[objects[i].material].shininess);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, objects[i].diffuseTexID);
			break;
		case ShadingType::SKYBOX:
			glUseProgram(shaders_IDs[SKYBOX]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[SKYBOX].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, objects[i].diffuseTexID);
			// We draw the skybox first, so if we disable writes on the Z-BUFFER, 
			// all later draw calls will overwrite the skybox pixels for sure, 
			// no matter how distant is the skybox.
			glDepthMask(GL_FALSE);
			break;
		case ShadingType::REFLECTION:
			//TODO REFLECTION
			break;
		case ShadingType::REFRACTION:
			// TODO REFRACTION
			break;
		case ShadingType::TEXTURE_ONLY:
			glUseProgram(shaders_IDs[TEXTURE_ONLY]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[TEXTURE_ONLY].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, objects[i].diffuseTexID);
			break;
		case ShadingType::PASS_THROUGH:
			glUseProgram(shaders_IDs[PASS_THROUGH]);
			// Caricamento matrice trasformazione del modello
			glUniformMatrix4fv(base_uniforms[PASS_THROUGH].M_Matrix_pointer, 1, GL_FALSE, value_ptr(objects[i].M));
			break;
		default:
			break;
		}
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		glBindVertexArray(objects[i].mesh.vertexArrayObjID);
		glDrawArrays(GL_TRIANGLES, 0, objects[i].mesh.vertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDepthMask(GL_TRUE);
	}

	// Transparent objects reordering
	// Draw in a reversed order: from farest to nearest
	// TODO RENDER TRANSPARENT OBJECTS 

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
		glUniform3fv(base_uniforms[i].camera_position_pointer, 1, glm::value_ptr(glm::vec3(ViewSetup.position)));
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
	float amount = 0.1f;
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
		modifyModelMatrix(glm::vec3(0), axis, amount, 1.0f);
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
	case 'W': TorusSetup.NumWraps++;
		compute_Torus(&(objects[TorusSetup.torus_index].mesh));
		generate_and_load_buffers(false, &(objects[TorusSetup.torus_index].mesh));
		break;
	case 'w':
		if (TorusSetup.NumWraps > 4) {
			TorusSetup.NumWraps--;
			compute_Torus(&(objects[TorusSetup.torus_index].mesh));
			generate_and_load_buffers(false, &(objects[TorusSetup.torus_index].mesh));
		}
		break;
	case 'N':
		TorusSetup.NumPerWrap++;
		compute_Torus(&(objects[TorusSetup.torus_index].mesh));
		generate_and_load_buffers(false, &(objects[TorusSetup.torus_index].mesh));
		break;
	case 'n':
		if (TorusSetup.NumPerWrap > 4) {
			TorusSetup.NumPerWrap--;
			compute_Torus(&(objects[TorusSetup.torus_index].mesh));
			generate_and_load_buffers(false, &(objects[TorusSetup.torus_index].mesh));
		}
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
		selected_obj = selected_obj > 0 ? selected_obj - 1 : movables.size() - 1;
		ViewSetup.target = objects[movables[selected_obj]].M * glm::vec4(0.f, 0.f, 0.f, 1.f);
		break;
	case GLUT_KEY_RIGHT:
		selected_obj = (selected_obj + 1) < movables.size() ? selected_obj + 1 : 0;
		ViewSetup.target = objects[movables[selected_obj]].M * glm::vec4(0.f, 0.f, 0.f, 1.f);
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
	objects[movables[selected_obj]].material = (MaterialType)option;
}

void buildOpenGLMenu()
{
	int materialSubMenu = glutCreateMenu(material_menu_function);

	glutAddMenuEntry(materials[MaterialType::RED_PLASTIC].name.c_str(), MaterialType::RED_PLASTIC);
	glutAddMenuEntry(materials[MaterialType::EMERALD].name.c_str(), MaterialType::EMERALD);
	glutAddMenuEntry(materials[MaterialType::BRASS].name.c_str(), MaterialType::BRASS);
	glutAddMenuEntry(materials[MaterialType::SLATE].name.c_str(), MaterialType::SLATE);

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
	ViewSetup.position += direction * CAMERA_ZOOM_SPEED;
}

void moveCameraBack()
{
	glm::vec4 direction = ViewSetup.target - ViewSetup.position;
	ViewSetup.position -= direction * CAMERA_ZOOM_SPEED;
}

void moveCameraLeft()
{
	// SEE Lab_03
}

void moveCameraRight()
{
	// SEE Lab_03
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

void modifyModelMatrix(glm::vec3 translation_vector, glm::vec3 rotation_vector, GLfloat angle, GLfloat scale_factor)
{
	// SEE LAB_03 se volete potete inserire i tool di trasformazione già implementati 	
}

/*
 * Computes the Vertex attributes data for segment number j of wrap number i.
 */
void computeTorusVertex(int i, int j, Mesh* mesh) {

	float pig = 1.f * glm::pi<float>();
	float theta = 2.f * glm::pi<float>() * (float)i / (float)TorusSetup.NumWraps;
	float phi = 2.f * glm::pi<float>() * (float)j / (float)TorusSetup.NumPerWrap;
	float sinphi = sin(phi);
	float cosphi = cos(phi);
	float sintheta = sin(theta);
	float costheta = cos(theta);

	float tmp = TorusSetup.MajorRadius + TorusSetup.MinorRadius * cosphi;
	float x = sintheta * tmp;
	float y = TorusSetup.MinorRadius * sinphi;
	float z = costheta * tmp;

	mesh->vertices.push_back(glm::vec3(x, y, z));
	mesh->normals.push_back(glm::vec3(sintheta * cosphi, sinphi, costheta * cosphi));
	mesh->texCoords.push_back(glm::vec2(theta / pig, phi / pig));
	}

void compute_Torus(Mesh* mesh)
{
	mesh->vertices.clear();
	mesh->normals.clear();
	mesh->texCoords.clear();

	// draw the torus as NumWraps strips one next to the other
	for (int i = 0; i < TorusSetup.NumWraps; i++) {
		for (int j = 0; j <= TorusSetup.NumPerWrap; j++) {
			// first face   3
			//				| \
			//				1--2
			computeTorusVertex(i, j, mesh);
			computeTorusVertex(i + 1, j, mesh);
			computeTorusVertex(i, j + 1, mesh);
			// second face  1--3
			//				 \ |
			//				   2
			computeTorusVertex(i, j + 1, mesh);
			computeTorusVertex(i + 1, j, mesh);
			computeTorusVertex(i + 1, j + 1, mesh);
		}
	}
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
		// Genero 1 Buffer Object per le coordinate texture
		glGenBuffers(1, &mesh->uvBufferObjID);
		// Genero 1 Buffer Object per le tangenti ai vertici (normal Maps)
		glGenBuffers(1, &mesh->tgBufferObjID);
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

	glBindBuffer(GL_ARRAY_BUFFER, mesh->uvBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, mesh->texCoords.size() * sizeof(glm::vec2), mesh->texCoords.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,					// attribute index in the shader
		2,                  // size
		GL_FLOAT,           // type
		false,              // normalized 
		0,					// stride
		(void*)0            // array buffer offset
	);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->tgBufferObjID);
	glBufferData(GL_ARRAY_BUFFER, mesh->tangents.size() * sizeof(glm::vec3), mesh->tangents.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,					// attribute index in the shader
		3,                  // size
		GL_FLOAT,           // type
		false,              // normalized 
		0,					// stride
		(void*)0            // array buffer offset
	);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
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

	// If normals and uvs are not loaded, we calculate them for a default smooth shading effect
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
			tmp_normals[i / 3] = normal;
			//Put an index to the normal for all 3 vertex of the face
			normalIndices.push_back(i / 3);
			normalIndices.push_back(i / 3);
			normalIndices.push_back(i / 3);
		}
	}
	//if texture coordinates were not included we fake them
	if (tmp_uvs.size() == 0) {
		tmp_uvs.push_back(glm::vec2(0)); //dummy uv
		for (int i = 0; i < vertexIndices.size(); i += 3)
		{
			// The UV is dummy
			uvIndices.push_back(0);
			uvIndices.push_back(0);
			uvIndices.push_back(0);
		}
	}
	// We prepare the data for glDrawArrays calls, this is a simple but non optimal way of storing mesh data.
	// However, you could optimize the mesh data using a index array for both vertex positions, 
	// normals and textures and later use glDrawElements
	int i = 0;
	// Now following the index arrays, we build the final arrays that will contain the data for glDrawArray...
	for (int i = 0; i < vertexIndices.size(); i++) {

		mesh->vertices.push_back(tmp_vertices[vertexIndices[i]]);
		mesh->normals.push_back(tmp_normals[normalIndices[i]]);
		mesh->texCoords.push_back(tmp_uvs[uvIndices[i]]);
	}
}

GLuint loadTexture(string path)
{
	GLint width, height, texChannels, format;
	GLuint textureID;
	stbi_uc* pixels = stbi_load(path.data(), &width, &height, &texChannels, 0);
	if (!pixels) {
		std::cerr << "\nFailed to load texture image! --> " << path << std::endl;
		std::getchar();
		exit(EXIT_FAILURE);
	}
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Texture displacement logic
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//Texture sampling logic
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	switch (texChannels) {
	case 1:	format = GL_RED;
		break;
	case 3: format = GL_RGB;
		break;
	case 4: format = GL_RGBA;
		break;
	default: format = GL_RGB;
		break;
	}

	// data loading in memory
	glTexImage2D(GL_TEXTURE_2D,  //the target
		0, // the mip map level we want to generate
		format,
		width,
		height,
		0, // border, leave 0
		format, // we assume is a RGB color image with 24 bit depth per pixel
		GL_UNSIGNED_BYTE, // the data type
		pixels);
	glGenerateMipmap(GL_TEXTURE_2D);// automatic mip maps generation

	stbi_image_free(pixels);
	return textureID;
}

GLuint load_cube_map_texture(string face_textures[6])
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, channels;
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char* data = stbi_load(face_textures[i].c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);// automatic mip maps generation
			stbi_image_free(data);
		}
		else
		{
			std::cerr << "\nFailed to load texture image! --> " << face_textures[i] << std::endl;
			std::getchar();
			exit(EXIT_FAILURE);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void calc_tangents(Mesh* mesh)
{
	for (int i = 0; i < mesh->vertices.size(); i += 3) {
		glm::vec3 tangent;
		glm::vec3 edge1 = mesh->vertices[i + 1] - mesh->vertices[i];
		glm::vec3 edge2 = mesh->vertices[i + 2] - mesh->vertices[i];
		glm::vec2 deltaUV1 = mesh->texCoords[i + 1] - mesh->texCoords[i];
		glm::vec2 deltaUV2 = mesh->texCoords[i + 2] - mesh->texCoords[i];
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
		tangent = glm::normalize(tangent);
		mesh->tangents.push_back(tangent);
		mesh->tangents.push_back(tangent);
		mesh->tangents.push_back(tangent);
	}
}

void printToScreen()
{
	string axis = "Axis: ";
	string mode = "Navigate/Modify: ";
	string obj = "Object: " + objects[movables[selected_obj]].name;
	string ref = "WCS/OCS: ";
	string shader = "Shader: ";
	switch (objects[movables[selected_obj]].shading) {
	case ShadingType::NORMAL_MAPPING:shader += "NORMAL_MAPPING";
		break;
	case ShadingType::TEXTURE_PHONG:shader += "TEXTURE_PHONG";
		break;
	case ShadingType::TEXTURE_ONLY:shader += "TEXTURE_ONLY";
		break;
	case ShadingType::REFLECTION:shader += "REFLECTION";
		break;
	case ShadingType::REFRACTION:shader += "REFRACTION";
		break;
	case ShadingType::PASS_THROUGH:shader += "PASS_THROUGH";
		break;
	case ShadingType::SKYBOX:shader += "SKYBOX";
		break;
	default:
		break;
	}
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
	lines.push_back(shader);
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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Model Viewer Texture|Normal|Env Mapping");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboardDown);
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseActiveMotion);
	glutSpecialFunc(special);

	glewExperimental = GL_TRUE;
	GlewInitResult = glewInit();
	if (GLEW_OK != GlewInitResult) {
		fprintf(
			stderr,
			"ERROR: %s\n",
			glewGetErrorString(GlewInitResult)
		);
		exit(EXIT_FAILURE);
	}
	fprintf(
		stdout,
		"INFO: OpenGL Version: %s\n",
		glGetString(GL_VERSION)
	);

	init();
	initShader();

	buildOpenGLMenu();
	refresh_monitor(32);
	glutMainLoop();

	return 1;
}
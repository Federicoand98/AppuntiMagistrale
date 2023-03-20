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
#include <GLFW/glfw3.h>

// Include GLM; libreria matematica per le opengl
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int		width = 800;
int		height = 800;
int		fov = 45;
float	zNear = 0.01;
float	zFar = 10000.0;
int assi = 2;

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

void polygon(int a, int b, int c, int d, int color) {
    vColors[Index] = colors[color]; vPositions[Index] = positions[a]; Index++;
    vColors[Index] = colors[color]; vPositions[Index] = positions[b]; Index++;
    vColors[Index] = colors[color]; vPositions[Index] = positions[c]; Index++;
    vColors[Index] = colors[color]; vPositions[Index] = positions[a]; Index++;
    vColors[Index] = colors[color]; vPositions[Index] = positions[c]; Index++;
    vColors[Index] = colors[color]; vPositions[Index] = positions[d]; Index++;
}

void colorcube() {
    polygon(1, 0, 3, 2, 0);
    polygon(2, 3, 7, 6, 1);
    polygon(3, 0, 4, 7, 2);
    polygon(6, 5, 1, 2, 3);
    polygon(4, 5, 6, 7, 4);
    polygon(5, 4, 0, 1, 5);
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

float sc[] = {
        1.0f, 1.0f, 1.0f
};

void initShader(void) {
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexShader_C.glsl";
    char* fragmentShader = (char*)"fragmentShader_C.glsl";

    programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
    glUseProgram(programId);
}

void init(void) {
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

    //Definisco il colore che verr� assegnato allo schermo
    glClearColor(0.0, 0.0, 0.0, 0.0);

    // Ottieni l'identificativo della variabile uniform mat4 Projection (in vertex shader).
    //Questo identificativo sar� poi utilizzato per il trasferimento della matrice Projection al Vertex Shader
    MatProj = glGetUniformLocation(programId, "Projection");
    // Ottieni l'identificativo della variabile uniform mat4 Model (in vertex shader)
    //Questo identificativo sar� poi utilizzato per il trasferimento della matrice Model al Vertex Shader
    MatModel = glGetUniformLocation(programId, "Model");
    //Ottieni l'identificativo della variabile uniform mat4 View (in vertex shader)
    //Questo identificativo sar� poi utilizzato per il trasferimento della matrice View al Vertex Shader
    MatView = glGetUniformLocation(programId, "View");
}

void processInput(GLFWwindow* window) {
    float alfa = 0.05 * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        rotateY -= 10;
    else if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        rotateY += 10;
    else if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        rotateX += 10;
    else if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        rotateX -= 10;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        assi = 0;
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        assi = 1;
    if(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
        assi = 2;

    sc[assi] *= 1.1f;

    if(sc[assi] > 15.0)
        sc[assi] = 1.0f;
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (yoffset > 0)
        fov += 1;
    else
        fov -= 1;
}

void drawScene(GLFWwindow* window) {
    int i, n_cubi = 1;  // fino a 10 cubi
    //float timevalue = glutGet(GLUT_ELAPSED_TIME) * 0.0001;
    float timevalue = glfwGetTime() * 0.0001;

    //Passo al Vertex Shader il puntatore alla matrice Projection, che sar� associata alla variabile Uniform mat4 Projection
    //all'interno del Vertex shader. Uso l'identificatio MatProj
    Projection = perspective(radians((float)fov), (float)(width) / float(height), zNear, zFar);
    glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
    //Costruisco la matrice di Vista che applicata ai vertici in coordinate mondo WCS
    //li trasforma nel sistema di riferimento della camera VCS.
    // usage: lookAt(eye,at,up);
    View = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(MatView, 1, GL_FALSE, value_ptr(View));

    //float currentFrame = glutGet(GLUT_ELAPSED_TIME);
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - last_frame;
    last_frame = currentFrame;
    glClearColor(1.0, 0.0, 1.0, 0.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (i = 0; i < n_cubi; i++) {
        Model = mat4(1.0);
        Model = translate(Model, cubePositions[i]);
        Model = rotate(Model, radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f));
        Model = rotate(Model, radians(rotateY), glm::vec3(0.0f, 1.0f, 0.0f));
        Model = rotate(Model, radians(rotateZ), glm::vec3(0.0f, 0.0f, 1.0f));

        //Model = scale(Model, vec3(2.0f, 2.0f, 2.0f));
        Model = scale(Model, vec3(sc[0], sc[1], sc[2]));

        //Passo al Vertex Shader il puntatore alla matrice Model, che sar� associata alla variabile Uniform mat4 Projection
        //all'interno del Vertex shader. Uso l'identificatio MatModel
        glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));

        glBindVertexArray(vao);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main(int argc, char* argv[]) {
    GLFWwindow* window;

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    std::cout << "I'm apple machine" << std::endl;
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow( 800, 800, "3D CUBE", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glViewport(0, 0, 800, 800);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetKeyCallback(window, keyboard);

    std::cout << "openGl version: " << glGetString(GL_VERSION) << std::endl;

    if(glewInit() != GLEW_OK) {
        std::cout << "ERROR!" << std::endl;
    }

    // vari init
    initShader();
    init();

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        drawScene(window);
    }

    glfwTerminate();
    return 0;
}
// 2D_TRIANGLE.cpp :
//

// 2D_TRIANGLE.cpp :
//
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include "ShaderMaker.h"

static unsigned int programId;

unsigned int VAO, VAO_2;
unsigned int VBO, VBO_2;

using namespace glm;

typedef struct { float x, y, z; } Point;

int nPoint = 100000;
Point* Punti = new Point[nPoint];

int nPointPOL = 5;
Point* PuntiPOL = new Point[nPointPOL];

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        glClearColor(1.0, 0.0, 0.0, 1.0);
}

void sierpinski(void) {
    Point T[3] = { {-0.5f, -0.5f, 0.0f} , {0.5f, -0.5f, 0.0f} , {0.0f, 0.5f, 0.0f} };

    int i, index = rand() % 3;
    // Scegliamo il punto iniziale  , a caso tra uno dei tre  vertici
    Punti[0] = T[index];

    for (i = 1; i < nPoint; i++)
    {
        //Scelgo a caso uno dei tre vertici iniziali
        index = rand() % 3;
        //Il nuovo punto � il punto medio tra il punto precedente
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

void drawScene(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT);

    // cose
    glBindVertexArray(VAO);
    glPointSize(4.0);
    glDrawArrays(GL_POINTS, 0, nPoint);

    glBindVertexArray(VAO_2);
    glLineWidth(2.5);
    glDrawArrays(GL_LINE_STRIP, 0, nPointPOL);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main(int argc, char* argv[]) {
    GLFWwindow* window;

    if (!glfwInit()) {
        std::cout << "Error glfwInit()" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    std::cout << "I'm apple machine" << std::endl;
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow( 800, 800, "Sierpinsky 2D", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glViewport(0, 0, 800, 800);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, keyboard);

    std::cout << "openGl version: " << glGetString(GL_VERSION) << std::endl;

    if(glewInit() != GLEW_OK) {
        std::cout << "ERROR!" << std::endl;
    }

    // vari init
    initShader();
    init();

    while (!glfwWindowShouldClose(window)) {
        drawScene(window);
    }

    glfwTerminate();
    return 0;
}
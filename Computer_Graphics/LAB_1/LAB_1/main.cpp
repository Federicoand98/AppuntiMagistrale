// 2D_TRIANGLE.cpp :
//

// 2D_TRIANGLE.cpp :
//
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
// Include GLM; libreria matematica per le opengl
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static unsigned int programId;

unsigned int VAO;
unsigned int VBO;

unsigned int VAO_2;
unsigned int VBO_2;

using namespace glm;

#define MaxNumPts 300
float PointArray[MaxNumPts][2];
float CurveArray[MaxNumPts][2];

int NumPts = 0;

// Window size in pixels
int		width = 500;
int		height = 500;

// Drag variables
float delta = 0.02;
int idx = 0;
bool drag = false;

/* Prototypes */
void addNewPoint(float x, float y);
int main(int argc, char** argv);
void removeFirstPoint();
void removeLastPoint();

void framebuffer_size_callback(GLFWwindow* window, int Width, int Height) {
    height = (Height > 1) ? Height : 2;
    width = (Width > 1) ? Width : 2;
    glViewport(0, 0, width, height);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if(key == GLFW_KEY_F && action == GLFW_PRESS)
        removeFirstPoint();
    else if(key == GLFW_KEY_L && action == GLFW_PRESS)
        removeLastPoint();
}

void mouse_drag_callback(GLFWwindow* window, double x, double y) {
    if(drag) {
        float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
        float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

        PointArray[idx][0] = xPos;
        PointArray[idx][1] = yPos;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    double xMouse, yMouse;
    glfwGetCursorPos(window, &xMouse, &yMouse);

    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        float xPos = -1.0f + ((float)xMouse) * 2 / ((float)(width));
        float yPos = -1.0f + ((float)(height - yMouse)) * 2 / ((float)(height));
        bool trovato = false;

        for(int i = 0; i < NumPts && !trovato; i++) {
            if(xPos <= PointArray[i][0] + delta && xPos >= PointArray[i][0] - delta &&
               yPos <= PointArray[i][1] + delta && yPos >= PointArray[i][1] - delta) {
                trovato = true;
                drag = true;
                idx = i;
            }
        }

        if(!trovato) {
            addNewPoint(xPos, yPos);
        }
    }

    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && drag) {
        float xPos = -1.0f + ((float)xMouse) * 2 / ((float)(width));
        float yPos = -1.0f + ((float)(height - yMouse)) * 2 / ((float)(height));

        PointArray[idx][0] = xPos;
        PointArray[idx][1] = yPos;

        idx = 0;
        drag = false;
    }
}

void mousewheel(GLFWwindow *window, double xoffset, double yoffset) {
    // TODO
    // da riempire con la logica della rotella del mouse
    // la yoffset gestisce la direzione di scroll della rotella
}

// Add a new point to the end of the list.
// Remove the first point in the list if too many points.
void removeLastPoint() {
    if (NumPts > 0) {
        NumPts--;
    }
}

// Add a new point to the end of the list.
// Remove the first point in the list if too many points.
void addNewPoint(float x, float y) {
    if (NumPts >= MaxNumPts) {
        removeFirstPoint();
    }
    PointArray[NumPts][0] = x;
    PointArray[NumPts][1] = y;
    NumPts++;
}

void removeFirstPoint() {
    int i;
    if (NumPts > 0) {
        // Remove the first point, slide the rest down
        NumPts--;
        for (i = 0; i < NumPts; i++) {
            PointArray[i][0] = PointArray[i + 1][0];
            PointArray[i][1] = PointArray[i + 1][1];
        }
    }
}

void initShader(void) {
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexShader.glsl";
    char* fragmentShader = (char*)"fragmentShader.glsl";

    programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
    glUseProgram(programId);

}

void init(void) {
    // VAO for control polygon
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // VAO for curve
    glGenVertexArrays(1, &VAO_2);
    glBindVertexArray(VAO_2);
    glGenBuffers(1, &VBO_2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_2);

    // Background color
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glViewport(0, 0, 500, 500);
}

void deCasteljau(float t, float* result) {
    float temp1[MaxNumPts][2];

    for(int i = 0; i < NumPts; i++) {
        temp1[i][0] = PointArray[i][0];
        temp1[i][1] = PointArray[i][1];
    }

    for(int i = 1; i < NumPts; i++) {
        for(int j = 0; j < NumPts - i; j++) {
            temp1[j][0] = (1- t) * temp1[j][0] + t * temp1[j + 1][0];
            temp1[j][1] = (1- t) * temp1[j][1] + t * temp1[j + 1][1];
        }
    }

    result[0] = temp1[0][0];
    result[1] = temp1[0][1];
}

void drawScene(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT);

    if (NumPts > 1) {
        float result[2];

        for(int i = 0; i <= 100; i++) {
            deCasteljau((GLfloat)i / 100, result);
            CurveArray[i][0] = result[0];
            CurveArray[i][1] = result[1];
        }

        glBindVertexArray(VAO_2);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glPointSize(1.0);
        glLineWidth(2.5);
        glDrawArrays(GL_LINE_STRIP, 0, 101);
        glBindVertexArray(0);
    }

    // Draw control polygon
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PointArray), &PointArray[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Draw the control points CP
    glPointSize(15.0);
    glDrawArrays(GL_POINTS, 0, NumPts);
    // Draw the line segments between CP
    glLineWidth(12.0);
    glDrawArrays(GL_LINE_STRIP, 0, NumPts);
    glBindVertexArray(0);

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

    window = glfwCreateWindow( width, height, "LAB - 01", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, keyboard);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, mousewheel);
    glfwSetCursorPosCallback(window, mouse_drag_callback);

    std::cout << "openGl version: " << glGetString(GL_VERSION) << std::endl;

    if(glewInit() != GLEW_OK) {
        std::cout << "ERROR!" << std::endl;
    }

    initShader();
    init();

    while (!glfwWindowShouldClose(window)) {
        drawScene(window);
    }

    glfwTerminate();
    return 0;
}
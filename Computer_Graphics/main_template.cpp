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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        glClearColor(1.0, 0.0, 0.0, 1.0);

    // TODO
    // da riempire con altri if else
}

void mousewheel(GLFWwindow *window, double xoffset, double yoffset) {
    // TODO
    // da riempire con la logica della rotella del mouse
    // la yoffset gestisce la direzione di scroll della rotella
}

void drawScene(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT);

    //TODO
    // da incollare la drawScene della prof ad esclusione della funzione
    // glutSwapBuffers() e fare attenzione ad eventuali gluGet(ELAPSET_TIME) che va cambiata
    // in glfwGetTime()

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

    window = glfwCreateWindow( 800, 800, "Hello World", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glViewport(0, 0, 800, 800);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, keyboard);
    glfwSetScrollCallback(window, mousewheel);

    std::cout << "openGl version: " << glGetString(GL_VERSION) << std::endl;

    if(glewInit() != GLEW_OK) {
        std::cout << "ERROR!" << std::endl;
    }

    // vari init della prof
    // quindi init() e initShader()


    

    while (!glfwWindowShouldClose(window)) {
        drawScene(window);
    }

    glfwTerminate();
    return 0;
}
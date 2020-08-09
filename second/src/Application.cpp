#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Uniforms.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

#define glCall(x) glClearErrors(); x; if (!glCheckErrors(#x, __FILE__, __LINE__)) __debugbreak();

using namespace GLTools;

void glClearErrors() {
    while (glGetError() != GL_NO_ERROR);
}

bool glCheckErrors(const char* call, const char* file, int line) {
    unsigned int err;
    bool ok = true;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "[opengl error] (" << err << ")" << ": " << call << " in " << file << " on line " << line << std::endl;
        ok = false;
    }
    return ok;
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;
    
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    
    glfwSwapInterval(1);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "oh fuck" << std::endl;
        return -1;
    };

    std::cout << glGetString(GL_VERSION) << std::endl;
    float points[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f
    };

    float points2[] = {
        -0.5f, -0.3f,
        1, -1,
        1, 1,
        -0.5f, 0.3f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), points, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    unsigned int buf2;
    glGenBuffers(1, &buf2);
    glBindBuffer(GL_ARRAY_BUFFER, buf2);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), points2, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);


    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    Shader sh1;
    sh1.setVertexFromFile("src/shaders/basic1/shader.vert");
    sh1.setFragmentFromFile("src/shaders/basic1/shader.frag");
    sh1.compile();

    Shader sh2;
    sh2.setVertexFromFile("src/shaders/basic2/shader.vert");
    sh2.setFragmentFromFile("src/shaders/basic2/shader.frag");
    sh2.compile();
    
    //sh1.use();
    sh2.use();

    UniformVec4 color(&sh2, "colorIn");
   // UniformFloat shadowBlur(&sh2, "shadowBlur");

    float cur = 0.0f;
    float add = 0.001f;
    glEnable(GL_BLEND);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        /*int w, h;
        glfwGetWindowSize(window, &w, &h);*/
        
        color.set(1, 0.5, 0.3, 1);
        
        glBindBuffer(GL_ARRAY_BUFFER, buf2);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        color.set(0.1, 1, 0.3, .2);
        //shadowBlur.set(0.1f);
        glBindBuffer(GL_ARRAY_BUFFER, buf);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));


        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
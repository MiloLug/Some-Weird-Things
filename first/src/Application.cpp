#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

#define glCall(x) glClearErrors(); x; if (!glCheckErrors(#x, __FILE__, __LINE__)) __debugbreak();

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

struct ShaderSet {
    std::string vertex = "";
    std::string fragment = "";
};

ShaderSet getShaders(const std::string& path) {
    std::ifstream is(path);
    enum Modes {
        vertex = 0,
        fragment = 1,
        none = -1
    };
    int mode = Modes::none;
    std::string shaders[2] = {"", ""};
    std::string tmp;
    while (std::getline(is, tmp)) {
        if (tmp.find("#shader") != std::string::npos) {
            if (tmp.find("vertex") != std::string::npos) {
                mode = Modes::vertex;
            }
            else if (tmp.find("fragment") != std::string::npos) {
                mode = Modes::fragment;
            }
            else {
                mode = Modes::none;
            }
        }
        else if(mode != Modes::none){
            shaders[mode] += tmp + "\n";
        }
    }
    return { shaders[Modes::vertex], shaders[Modes::fragment] };
}

unsigned int compileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char *str = source.c_str();
    glShaderSource(id, 1, &str, nullptr);
    glCompileShader(id);

    int status;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);    
    if (status != GL_TRUE) {
        int len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        char* log = new char[len];
        glGetShaderInfoLog(id, len, &len, log);
        
        std::cout << "shader bruh says: " << std::endl << log << std::endl;
        glDeleteShader(id);
        delete[] log;
        return -1;
    }

    return id;
}

unsigned int createShader(const std::string& vertex, const std::string& fragment) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertex);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragment);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;
    
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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
        -0.5f, -0.5f,   1.0f, 0.1f, 0.3f,
        0.5f, -0.5f,     1.0f, 0.1f, 0.3f,
        0.5f, 0.5f,    1.0f, 0.1f, 0.3f,
        -0.5f, 0.5f,     1.0f, 0.1f, 0.3f,

        0.9f, 1.0f,     1.0f, 0.1f, 0.3f,
        1.0f, 1.0f,     1.0f, 0.1f, 0.3f,
        1.0f, -1.0f,     1.0f, 0.1f, 0.3f,
        0.9f, -1.0f,     1.0f, 0.1f, 0.3f,
        
        -0.9f, 1.0f,     1.0f, 0.1f, 0.3f,
        -1.0f, 1.0f,     1.0f, 0.1f, 0.3f,
        -1.0f, -1.0f,     1.0f, 0.1f, 0.3f,
        -0.9f, -1.0f,     1.0f, 0.1f, 0.3f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        4, 6, 7,

        8, 9, 10,
        8, 10, 11
    };

    unsigned int buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, 60 * sizeof(float), points, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 18 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    ShaderSet shaders = getShaders("src/shaders/basic.shd");
    unsigned int prog = createShader(shaders.vertex, shaders.fragment);
    glUseProgram(prog);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        /*int w, h;
        glfwGetWindowSize(window, &w, &h);*/
        
        glCall(glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, nullptr));

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
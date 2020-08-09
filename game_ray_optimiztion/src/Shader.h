#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>
#include <fstream>
#include <sstream>

namespace GLTools {
    class Shader
    {
    private:
        int prog_id = -1;

        int sh_vertex = -1, sh_fragment = -1, sh_geometry = -1;

        bool compiled = false;

        static inline int compileShader(unsigned int type, const char* source) {
            unsigned int id = glCreateShader(type);
            glShaderSource(id, 1, &source, nullptr);
            glCompileShader(id);

            int status;
            glGetShaderiv(id, GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE) {
                int len;
                glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
                char* log = new char[len];
                std::string slog;
                glGetShaderInfoLog(id, len, &len, log);
                slog = log;
                delete[] log;
                glDeleteShader(id);
                throw std::runtime_error("GL: shader compilation failed, log:" + slog);
                return -1;
            }

            return id;
        }

        static inline void getShaderSourceFromFile(const char* path, std::string& ret) {
            std::ifstream shaderFile;
            shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            try
            {
                shaderFile.open(path);
                std::stringstream shaderStream;
                shaderStream << shaderFile.rdbuf();
                shaderFile.close();
                ret = shaderStream.str();
            }
            catch (std::ifstream::failure& e)
            {
                throw std::runtime_error("GL: shader file reading failed: " + std::string(path));
            }
        }

    public:
        Shader() {
            this->prog_id = glCreateProgram();
        }

        inline int getProgramId() {
            return this->prog_id;
        }
        inline bool isCompiled() {
            return this->compiled;
        }

        void setVertex(const char* source) {
            if (this->prog_id == -1)
                return;
            this->sh_vertex = Shader::compileShader(GL_VERTEX_SHADER, source);
            glAttachShader(this->prog_id, this->sh_vertex);
        }
        void setFragment(const char* source) {
            if (this->prog_id == -1)
                return;
            this->sh_fragment = Shader::compileShader(GL_FRAGMENT_SHADER, source);
            glAttachShader(this->prog_id, this->sh_fragment);
        }
        void setGeometry(const char* source) {
            if (this->prog_id == -1)
                return;
            this->sh_geometry = Shader::compileShader(GL_GEOMETRY_SHADER, source);
            glAttachShader(this->prog_id, this->sh_geometry);
        }

        void setVertexFromFile(const char* path) {
            std::string source;
            Shader::getShaderSourceFromFile(path, source);
            this->setVertex(source.data());
        }
        void setFragmentFromFile(const char* path) {
            std::string source;
            Shader::getShaderSourceFromFile(path, source);
            this->setFragment(source.data());
        }
        void setGeometryFromFile(const char* path) {
            std::string source;
            Shader::getShaderSourceFromFile(path, source);
            this->setGeometry(source.data());
        }

        void compile() {
            if (this->prog_id == -1)
                return;

            glLinkProgram(this->prog_id);

            int status;
            glGetProgramiv(this->prog_id, GL_LINK_STATUS, &status);
            if (status != GL_TRUE) {
                int len;
                glGetProgramiv(this->prog_id, GL_INFO_LOG_LENGTH, &len);
                char* log = new char[len];
                std::string slog;
                glGetProgramInfoLog(this->prog_id, len, &len, log);
                slog = log;
                delete[] log;
                glDeleteProgram(this->prog_id);
                this->prog_id = -1;
                throw std::runtime_error("GL: program linking failed, log:" + slog);
            }

            if (this->sh_fragment != -1)
                glDeleteShader(this->sh_fragment);
            if (this->sh_vertex != -1)
                glDeleteShader(this->sh_vertex);
            if (this->sh_geometry != -1)
                glDeleteShader(this->sh_geometry);

            this->compiled = true;
        }

        void use() {
            if (this->prog_id != -1 && this->compiled)
                glUseProgram(this->prog_id);
        }

        ~Shader() {
            if (this->prog_id != -1 && this->compiled)
                glDeleteProgram(this->prog_id);
        }
    };
}
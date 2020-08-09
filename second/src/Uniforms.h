#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include <stdexcept>

namespace GLTools
{
	class Uniform {
	protected:
		int prog_id;
		int uid;
	public:
		Uniform(Shader* shader, const char* name) {
			if (shader->isCompiled()) {
				this->prog_id = shader->getProgramId();
				this->uid = glGetUniformLocation(this->prog_id, name);
				if (this->uid == -1)
					throw std::runtime_error("GL: undefined uniform");
			};
		}
	};

	class UniformInt : public Uniform {
	public:
		UniformInt(Shader* shader, const char* name) : Uniform(shader, name) {};
		inline void set(int value) {
			glUniform1i(this->uid, value);
		}
	};
	class UniformFloat : public Uniform {
	public:
		UniformFloat(Shader* shader, const char* name) : Uniform(shader, name) {};
		inline void set(float value) {
			glUniform1f(this->uid, value);
		}
	};
	class UniformVec2 : public Uniform {
	public:
		UniformVec2(Shader* shader, const char* name) : Uniform(shader, name) {};
		inline void set(float x, float y) {
			glUniform2f(this->uid, x, y);
		}
	};
	class UniformVec3 : public Uniform {
	public:
		UniformVec3(Shader* shader, const char* name) : Uniform(shader, name) {};
		inline void set(float x, float y, float z) {
			glUniform3f(this->uid, x, y, z);
		}
	};
	class UniformVec4 : public Uniform {
	public:
		UniformVec4(Shader* shader, const char* name) : Uniform(shader, name) {};
		inline void set(float x, float y, float z, float w) {
			glUniform4f(this->uid, x, y, z, w);
		}
	};
};
#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include <stdexcept>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

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
		inline void set(const glm::vec2& value) {
			glUniform2fv(this->uid, 1, &value[0]);
		}
	};
	class UniformVec3 : public Uniform {
	public:
		UniformVec3(Shader* shader, const char* name) : Uniform(shader, name) {};
		inline void set(float x, float y, float z) {
			glUniform3f(this->uid, x, y, z);
		}
		inline void set(const glm::vec3& value) {
			glUniform3fv(this->uid, 1, &value[0]);
		}
	};
	class UniformVec4 : public Uniform {
	public:
		UniformVec4(Shader* shader, const char* name) : Uniform(shader, name) {};
		inline void set(float x, float y, float z, float w) {
			glUniform4f(this->uid, x, y, z, w);
		}
		inline void set(const glm::vec4& value) {
			glUniform4fv(this->uid, 1, &value[0]);
		}
	};
	class UniformMat2 : public Uniform {
	public:
		UniformMat2(Shader* shader, const char* name) : Uniform(shader, name) {};
		inline void set(const glm::mat2& value) {
			glUniformMatrix2fv(this->uid, 1, GL_FALSE, &value[0][0]);
		}
	};
	class UniformMat3 : public Uniform {
	public:
		UniformMat3(Shader* shader, const char* name) : Uniform(shader, name) {};
		inline void set(const glm::mat3& value) {
			glUniformMatrix3fv(this->uid, 1, GL_FALSE, &value[0][0]);
		}
	};
	class UniformMat4 : public Uniform {
	public:
		UniformMat4(Shader* shader, const char* name) : Uniform(shader, name) {};
		inline void set(const glm::mat4& value) {
			glUniformMatrix4fv(this->uid, 1, GL_FALSE, &value[0][0]);
		}
	};

}
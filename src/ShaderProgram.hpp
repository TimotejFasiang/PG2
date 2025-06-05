#pragma once

#include <memory>
#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include <GL/glew.h>

class ShaderProgram {
public:
	GLuint ID = 0;
	ShaderProgram() = default;

	static std::shared_ptr<ShaderProgram> create(const std::filesystem::path& vsPath,
											   const std::filesystem::path& fsPath);

	int getID() {
		return ID;
	}

	void activate() const;
	void clear();

	// Uniform setters
	void setUniform(const std::string& name, bool value) const;
	void setUniform(const std::string& name, int value) const;
	void setUniform(const std::string& name, float value) const;
	void setUniform(const std::string& name, const glm::vec2& value) const;
	void setUniform(const std::string& name, const glm::vec3& value) const;
	void setUniform(const std::string& name, const glm::vec4& value) const;
	void setUniform(const std::string& name, const glm::mat3& value) const;
	void setUniform(const std::string& name, const glm::mat4& value) const;

	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram& operator=(const ShaderProgram&) = delete;
	ShaderProgram(ShaderProgram&& other) noexcept;
	ShaderProgram& operator=(ShaderProgram&& other) noexcept;
	~ShaderProgram();

private:
	ShaderProgram(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath);
	GLuint compileShader(const std::filesystem::path& path, GLenum type);
	GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
	std::string readFile(const std::filesystem::path& path);
};
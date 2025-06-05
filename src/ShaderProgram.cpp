#include "ShaderProgram.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

std::shared_ptr<ShaderProgram> ShaderProgram::create(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath) {
    return std::shared_ptr<ShaderProgram>(new ShaderProgram(vsPath, fsPath));
}

ShaderProgram::ShaderProgram(const std::filesystem::path& vsPath,
                           const std::filesystem::path& fsPath) {
    GLuint vertexShader = compileShader(vsPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fsPath, GL_FRAGMENT_SHADER);
    ID = linkProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void ShaderProgram::activate() const {
    if (ID != 0) {  // Add validation check
        glUseProgram(ID);
    } else {
        std::cerr << "Warning: Attempted to activate invalid shader program" << std::endl;
    }
}

void ShaderProgram::clear() {
    glDeleteProgram(ID);
    ID = 0;
}

// Uniform setters (one implementation per type)
void ShaderProgram::setUniform(const std::string& name, bool value) const {
    if (ID != 0) {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
}

void ShaderProgram::setUniform(const std::string& name, int value) const {
    if (ID != 0) {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
}

void ShaderProgram::setUniform(const std::string& name, float value) const {
    if (ID != 0) {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec2& value) const {
    if (ID != 0) {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3& value) const {
    if (ID != 0) {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec4& value) const {
    if (ID != 0) {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat3& value) const {
    if (ID != 0) {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4& value) const {
    if (ID != 0) {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
    }
}

GLuint ShaderProgram::compileShader(const std::filesystem::path& path, GLenum type) {
    std::string source = readFile(path);
    const char* src = source.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Error checking
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed (" << path << "):\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint ShaderProgram::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Error checking
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    // Validation
    glValidateProgram(program);
    GLint valid;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &valid);
    if (!valid) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Shader program validation failed:\n" << log << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

std::string ShaderProgram::readFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file at: " << std::filesystem::absolute(path) << std::endl;
        throw std::runtime_error("Failed to open file: " + path.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : ID(other.ID) {
    other.ID = 0;  // Prevent double deletion
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        clear();
        ID = other.ID;
        other.ID = 0;
    }
    return *this;
}

ShaderProgram::~ShaderProgram() {
    clear();
}
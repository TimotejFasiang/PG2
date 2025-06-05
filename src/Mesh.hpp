#pragma once

#include <vector>
#include "assets.hpp"
#include "ShaderProgram.hpp"

class Mesh {
public:
    glm::vec3 getFirstVertexPosition() const {
        if (!vertices.empty()) return vertices[0].position;
        return glm::vec3(0.0f);
    }

    glm::vec3 getLastVertexPosition() const {
        if (!vertices.empty()) return vertices.back().position;
        return glm::vec3(0.0f);
    }

    float getMinY() const {
        if (vertices.empty()) return 0.0f;
        float minY = vertices[0].position.y;
        for (const auto& v : vertices) {
            minY = std::min(minY, v.position.y);
        }
        return minY;
    }

    float getMaxY() const {
        if (vertices.empty()) return 0.0f;
        float maxY = vertices[0].position.y;
        for (const auto& v : vertices) {
            maxY = std::max(maxY, v.position.y);
        }
        return maxY;
    }
    Mesh(GLenum primitiveType,
        std::shared_ptr<ShaderProgram> shader,
        const std::vector<vertex>& vertices,
        const std::vector<GLuint>& indices,
        glm::vec3 origin = glm::vec3(0.0f),
        glm::vec3 orientation = glm::vec3(0.0f));

    void draw();

    const std::vector<vertex>& getVertices() const { return vertices; }
    const std::vector<GLuint>& getIndices() const { return indices; }

private:
    std::shared_ptr<ShaderProgram> shader;
    GLuint VAO, VBO, EBO;
    std::vector<vertex> vertices;
    std::vector<GLuint> indices;
    GLenum primitiveType;
    glm::vec3 origin;
    glm::vec3 orientation;
};
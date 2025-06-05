#include "Mesh.hpp"
#include <iostream>

Mesh::Mesh(GLenum primitiveType, std::shared_ptr<ShaderProgram> shader, const std::vector<vertex>& vertices,
    const std::vector<GLuint>& indices, glm::vec3 origin, glm::vec3 orientation) : primitiveType(primitiveType),
    shader(std::move(shader)), vertices(vertices), indices(indices), origin(origin), orientation(orientation) {
    
    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &EBO);

    glNamedBufferStorage(VBO, vertices.size() * sizeof(vertex), vertices.data(), GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(EBO, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_STORAGE_BIT);

    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(vertex));
    glVertexArrayElementBuffer(VAO, EBO);

    // Position
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, position));
    glVertexArrayAttribBinding(VAO, 0, 0);

    // Normal
    glEnableVertexArrayAttrib(VAO, 1);
    glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, normal));
    glVertexArrayAttribBinding(VAO, 1, 0);

    // TexCoord
    glEnableVertexArrayAttrib(VAO, 2);
    glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, texcoord));
    glVertexArrayAttribBinding(VAO, 2, 0);

    // Verify attribute locations
    GLint maxAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);

    for (int i = 0; i < 3; ++i) {
        GLint enabled;
        glGetVertexArrayIndexediv(VAO, i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    }

    if (vertices.empty()) {
        throw std::runtime_error("Mesh created with empty vertices");
    }
    if (indices.empty()) {
        throw std::runtime_error("Mesh created with empty indices");
    }
}

void Mesh::draw() {
    if (shader) {
        shader->activate();
        glBindVertexArray(VAO);
        // glDrawElements(primitiveType, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}
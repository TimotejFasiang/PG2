#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Vertex description with position, texture coordinates, and normal
struct vertex {
    glm::vec3 position;  // vertex position (x, y, z)
    glm::vec3 normal;    // normal vector (x, y, z)
    glm::vec2 texcoord;  // texture coordinates (u, v)


    // Constructor for easy initialization
    vertex(glm::vec3 pos = {}, glm::vec3 norm = {}, glm::vec2 tex = {})
        : position(pos), normal(norm), texcoord(tex) {}

    // Get the stride for vertex attribute pointers
    static constexpr size_t stride() { return sizeof(vertex); }

    // Comparison operator
    bool operator==(const vertex& other) const {
        return position == other.position &&
                normal == other.normal &&
                texcoord == other.texcoord;
    }
};
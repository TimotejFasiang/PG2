#include "OBJloader.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <iostream>

struct VertexHash {
    size_t operator()(const vertex& v) const {
        size_t h1 = std::hash<float>()(v.position.x);
        size_t h2 = std::hash<float>()(v.position.y);
        size_t h3 = std::hash<float>()(v.position.z);
        size_t h4 = std::hash<float>()(v.normal.x);
        size_t h5 = std::hash<float>()(v.normal.y);
        size_t h6 = std::hash<float>()(v.normal.z);
        size_t h7 = std::hash<float>()(v.texcoord.x);
        size_t h8 = std::hash<float>()(v.texcoord.y);
        return h1 ^ h2 ^ h3 ^ h4 ^ h5 ^ h6 ^ h7 ^ h8;
    }
};

bool loadOBJ(const std::string& path,
            std::vector<vertex>& out_vertices,
            std::vector<GLuint>& out_indices) {

    // Clear output containers
    out_vertices.clear();
    out_indices.clear();

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::vector<glm::vec2> temp_texcoords;
    std::vector<GLuint> position_indices, normal_indices, texcoord_indices;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return false;
    }

    std::string line;
    size_t line_num = 0;
    try {
        while (std::getline(file, line)) {
            line_num++;
            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "v") {
                glm::vec3 pos;
                if (!(iss >> pos.x >> pos.y >> pos.z)) {
                    std::cerr << "Error reading vertex at line " << line_num << std::endl;
                    continue;
                }
                temp_positions.push_back(pos);
            }
            else if (type == "vn") {
                glm::vec3 norm;
                if (!(iss >> norm.x >> norm.y >> norm.z)) {
                    std::cerr << "Error reading normal at line " << line_num << std::endl;
                    continue;
                }
                temp_normals.push_back(norm);
            }
            else if (type == "vt") {
                glm::vec2 tex;
                if (!(iss >> tex.x >> tex.y)) {
                    std::cerr << "Error reading texcoord at line " << line_num << std::endl;
                    continue;
                }
                tex.y = 1.0f - tex.y; // Flip V coordinate
                temp_texcoords.push_back(tex);
            }
            else if (type == "f") {
                unsigned int v_idx, t_idx, n_idx;
                char slash;
                for (int i = 0; i < 3; ++i) {
                    if (!(iss >> v_idx >> slash >> t_idx >> slash >> n_idx)) {
                        std::cerr << "Error reading face at line " << line_num << std::endl;
                        break;
                    }
                    position_indices.push_back(v_idx - 1);
                    texcoord_indices.push_back(t_idx - 1);
                    normal_indices.push_back(n_idx - 1);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing OBJ file: " << e.what() << std::endl;
        return false;
    }

    // Validate indices
    if (position_indices.size() != normal_indices.size() ||
        position_indices.size() != texcoord_indices.size()) {
        std::cerr << "Mismatched index counts in OBJ file" << std::endl;
        return false;
    }

    // Check array bounds
    auto validate_index = [](size_t idx, size_t size, const char* type) {
        if (idx >= size) {
            std::cerr << "Invalid " << type << " index: " << idx
                     << " (max " << size-1 << ")" << std::endl;
            return false;
        }
        return true;
    };

    // Create indexed vertices
    std::unordered_map<vertex, GLuint, VertexHash> vertexMap;
    try {
        for (size_t i = 0; i < position_indices.size(); ++i) {
            if (!validate_index(position_indices[i], temp_positions.size(), "position") ||
                !validate_index(normal_indices[i], temp_normals.size(), "normal") ||
                !validate_index(texcoord_indices[i], temp_texcoords.size(), "texture")) {
                return false;
            }

            vertex v;
            v.position = temp_positions[position_indices[i]];
            v.normal = temp_normals[normal_indices[i]];
            v.texcoord = temp_texcoords[texcoord_indices[i]];

            if (vertexMap.find(v) == vertexMap.end()) {
                vertexMap[v] = static_cast<GLuint>(out_vertices.size());
                out_vertices.push_back(v);
            }
            out_indices.push_back(vertexMap[v]);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error creating vertex data: " << e.what() << std::endl;
        return false;
    }

    return true;
}
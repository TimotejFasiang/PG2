// Model.cpp
#include "Model.hpp"
#include "AnimatedTexture.hpp"
#include <iostream>
#include <OBJloader.hpp>
#include <memory>

Model::Model(const std::filesystem::path& path, 
             std::shared_ptr<ShaderProgram> shader)
    : shader(std::move(shader)) {
    
    std::vector<vertex> vertices;
    std::vector<GLuint> indices;

    if (!loadOBJ(path.string(), vertices, indices)) {
        throw std::runtime_error("Failed to load model: " + path.string());
    }

    if (vertices.empty() || indices.empty()) {
        throw std::runtime_error("Empty model data: " + path.string());
    }

    meshes.emplace_back(GL_TRIANGLES, this->shader, vertices, indices);
    name = path.stem().string();
}

float Model::getTransparency() const {
    return alpha;
}

bool Model::setTexture(const std::string& path) {
    texture = Texture::create(path);
    if (!texture || !texture->valid()) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return false;
        texture.reset();
    }
    return true;
}

bool Model::setAnimatedTexture(const std::string& path) {
    animatedTexture = std::make_unique<AnimatedTexture>();

    if (!animatedTexture->loadFromGif(path)) {
        animatedTexture.reset();
        return false;
    }

    isAnimated = true;
    return true;
}

void Model::draw() {
    if (!shader) return;
    
    shader->activate();
    
    // Set matrices and alpha
    shader->setUniform("alpha", alpha);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // X-axis
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Y-axis
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // Z-axis

    model = glm::scale(model, scale);
    shader->setUniform("model", model);
    
    // Handle color vs texture rendering
    if (useColor) {
        shader->setUniform("useTexture", 0); // Don't use texture
        shader->setUniform("objectColor", color); // Use the set color
    }
    else {
        // Texture handling, support both static and animated textures
        if (animatedTexture) {
            animatedTexture->bind(GL_TEXTURE0);
            shader->setUniform("useTexture", 1);
            shader->setUniform("diffuseTexture", 0);
        }
        else if (texture && texture->valid()) {
            texture->bind(GL_TEXTURE0);
            shader->setUniform("useTexture", 1);
            shader->setUniform("diffuseTexture", 0);
        } else {
            shader->setUniform("useTexture", 0);
            shader->setUniform("objectColor", glm::vec3(1.0f)); // Default white
        }
    }

    // Draw all meshes
    for (auto& mesh : meshes) {
        mesh.draw();
    }
}
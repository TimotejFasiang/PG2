// Model.hpp
#pragma once

#include <AnimatedTexture.hpp>
#include <filesystem>
#include <vector>
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"

class Model {

public:
    std::shared_ptr<Texture> texture;
    std::vector<Mesh> meshes;
    std::string name;
    glm::vec3 origin = glm::vec3(0.0f);
    glm::vec3 orientation = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    GLuint textureID = 0;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    std::unique_ptr<AnimatedTexture> animatedTexture;
    bool isAnimated = false;

    void setColor(const glm::vec3& color) {
        this->color = color;
        useColor = true;
    }

    bool transparent = false;

    bool hasAnimatedTexture() const {
        return isAnimated;
    }

    bool hasTransparency() const {
        // Transparent if either:
        // 1. The object has alpha < 1.0, OR
        // 2. It has a texture with alpha channel
        return alpha < 0.99f || (texture && texture->hasAlpha());
    }

    float alpha = 1.0f; // 1.0 = fully opaque, 0.0 = fully transparent

    float getTransparency() const;

    void setTransparency(float alphaValue) {
        alpha = glm::clamp(alphaValue, 0.0f, 1.0f);
        transparent = (alpha < 0.99f); // Consider slightly transparent as transparent
    }

    // Animations
    bool setAnimatedTexture(const std::string& path);
    void update(float deltaTime) {
        if (animatedTexture) {
            animatedTexture->update(deltaTime);
        }
    }

    Model() = default;

    Model(const std::filesystem::path& path, std::shared_ptr<ShaderProgram> shader);

    bool setTexture(const std::string& path);

    void draw();

private:
    std::shared_ptr<ShaderProgram> shader;
    bool useColor = false;
};
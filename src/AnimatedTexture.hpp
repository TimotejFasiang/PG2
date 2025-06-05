// AnimatedTexture.hpp
#pragma once
#include <string>
#include <vector>
#include <GL/glew.h>

class AnimatedTexture {
public:
    bool loadFromGif(const std::string& path);
    void update(float deltaTime);
    void bind(GLenum textureUnit) const;
    ~AnimatedTexture();

private:
    std::vector<GLuint> frames;
    std::vector<float> frameDelays;
    float currentTime = 0;
    size_t currentFrame = 0;
    bool loaded = false;
};
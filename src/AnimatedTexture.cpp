// AnimatedTexture.cpp
#include "AnimatedTexture.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

bool AnimatedTexture::loadFromGif(const std::string& path) {
    cv::VideoCapture cap(path);
    if (!cap.isOpened()) return false;

    int frameCount = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    frames.reserve(frameCount);
    frameDelays.reserve(frameCount);

    cv::Mat frame;
    while (cap.read(frame)) {
        // Convert to RGBA if needed
        if (frame.channels() == 3) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);
        }

        // Create OpenGL texture
        GLuint texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                    frame.cols, frame.rows, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, frame.data);

        frames.push_back(texID);
        frameDelays.push_back(1.0f / cap.get(cv::CAP_PROP_FPS)); // Simple timing
    }

    cap.release();
    loaded = !frames.empty();
    return loaded;
}

void AnimatedTexture::update(float deltaTime) {
    if (!loaded || frames.size() <= 1) return;

    currentTime += deltaTime;
    if (currentTime >= frameDelays[currentFrame]) {
        currentTime = 0;
        currentFrame = (currentFrame + 1) % frames.size();
    }
}

void AnimatedTexture::bind(GLenum textureUnit) const {
    if (!loaded) return;
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, frames[currentFrame]);
}

AnimatedTexture::~AnimatedTexture() {
    for (GLuint tex : frames) {
        glDeleteTextures(1, &tex);
    }
}
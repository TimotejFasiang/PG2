#include "Texture.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp> // For VideoCapture
#include <iostream>

std::shared_ptr<Texture> Texture::create(const std::string& path) {
    auto texture = std::shared_ptr<Texture>(new Texture());

    try {
        cv::Mat image;
        std::string extension = path.substr(path.find_last_of(".") + 1);

        // Handle GIF files
        if (extension == "gif" || extension == "GIF") {
            cv::VideoCapture cap(path);
            if (!cap.isOpened()) {
                throw std::runtime_error("Failed to open GIF file");
            }

            // Read first frame
            cap >> image;
            cap.release();

            if (image.empty()) {
                throw std::runtime_error("GIF frame is empty");
            }

            // Convert BGR to RGBA
            if (image.channels() == 3) {
                cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);
            } else if (image.channels() == 4) {
                cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA);
            } else {
                // Handle grayscale GIFs
                cv::cvtColor(image, image, cv::COLOR_GRAY2RGBA);
            }
        }
        else { // Handle other image formats normally
            // Load with alpha channel
            image = cv::imread(path, cv::IMREAD_UNCHANGED);
            if (image.empty()) {
                throw std::runtime_error("Failed to load image");
            }

            // Convert color space
            if (image.channels() == 4) {
                cv::cvtColor(image, image, cv::COLOR_BGRA2RGBA);
            } else if (image.channels() == 3) {
                cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
            } else if (image.channels() == 1) {
                cv::cvtColor(image, image, cv::COLOR_GRAY2RGB);
            }
        }

        // Error checking for unsupported channel counts
        if (image.channels() != 3 && image.channels() != 4) {
            throw std::runtime_error("Unsupported number of channels in texture");
        }

        // Determine texture format
        bool hasAlpha = (image.channels() == 4);
        GLenum format = hasAlpha ? GL_RGBA : GL_RGB;
        GLenum internalFormat = hasAlpha ? GL_RGBA8 : GL_RGB8;

        texture->m_width = image.cols;
        texture->m_height = image.rows;
        texture->m_hasAlpha = hasAlpha;

        // Generate texture with mipmaps
        glGenTextures(1, &texture->m_id);
        glBindTexture(GL_TEXTURE_2D, texture->m_id);
        if (texture->m_id == 0) {
            throw std::runtime_error("Failed to generate texture");
        }

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Enable anisotropic filtering if available
        if (GLEW_EXT_texture_filter_anisotropic) {
            float maxAniso;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
        }

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                    texture->m_width, texture->m_height,
                    0, format, GL_UNSIGNED_BYTE, image.data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

    } catch (const std::exception& e) {
        std::cerr << "Texture Error (" << path << "): " << e.what() << std::endl;
        if (texture->m_id != 0) {
            glDeleteTextures(1, &texture->m_id);
        }
        return nullptr;
    }

    return texture;
}

Texture::~Texture() {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
    }
}

void Texture::bind(GLenum textureUnit) const {
    if (m_id != 0) {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }
}
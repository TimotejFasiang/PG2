// Cube.hpp
#pragma once
#include "Model.hpp"

class Cube : public Model {
public:
    Cube(std::shared_ptr<ShaderProgram> shader, const std::string& texturePath);
};
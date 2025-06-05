// Cube.cpp

#include "Cube.hpp"
#include "Model.hpp"
#include <memory>

Cube::Cube(std::shared_ptr<ShaderProgram> shader, const std::string& texturePath)
    : Model("resources/objects/cube.obj", shader)
{
    setTexture(texturePath);
}

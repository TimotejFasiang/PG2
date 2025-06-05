#pragma once

#include <vector>
#include <string>
#include "assets.hpp"

bool loadOBJ(const std::string& path,
			std::vector<vertex>& out_vertices,
			std::vector<GLuint>& out_indices);
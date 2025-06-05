cmake_minimum_required(VERSION 3.30)
project(PG2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenCV REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)

# Add ImGui as subproject
add_subdirectory(external/imgui)

# Add executable
add_executable(PG2
        src/main.cpp
        src/app.cpp
        src/Camera.cpp
        src/ShaderProgram.cpp
        src/Model.cpp
        src/Mesh.cpp
        src/Texture.cpp
        src/Cube.cpp
        src/OBJloader.cpp
        src/gl_err_callback.cpp
        src/AnimatedTexture.cpp
)

# Link libraries
target_link_libraries(PG2 PRIVATE
        ${OpenCV_LIBS}
        GLEW::GLEW
        glfw
        OpenGL::GL
        glm::glm
        nlohmann_json::nlohmann_json
        imgui
        imgui_impl_glfw
        imgui_impl_opengl3
)

# Include directories
target_include_directories(PG2 PRIVATE
        src
        ${OpenCV_INCLUDE_DIRS}
)
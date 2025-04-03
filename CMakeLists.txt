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

# Add your executable and source files
add_executable(PG2 main.cpp app.cpp gl_err_callback.cpp ShaderProgram.cpp)

# Link libraries in the correct order
target_link_libraries(PG2
        ${OpenCV_LIBRARIES}
        ${GLEW_LIBRARIES}
        glfw  # Link GLFW (modern CMake target)
        OpenGL::GL  # Link OpenGL explicitly
        nlohmann_json::nlohmann_json
)

# Include directories using target-based approach
target_include_directories(PG2 PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${GLFW_INCLUDE_DIRS}
        ${OpenGL_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
        ${nlohmann_json_INCLUDE_DIRS}
)
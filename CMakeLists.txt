cmake_minimum_required(VERSION 3.30)  # Set CMake version to 3.30
project(PG2)  # Set project name to PG2

set(CMAKE_CXX_STANDARD 20)  # Use C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Enforce C++20

# Find and include OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Find and include GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# Find and include GLFW
find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

# Find and include OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})

# Find and include GLM (header-only, no linking required)
find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

# Find and include nlohmann/json
find_package(nlohmann_json REQUIRED)
include_directories(${nlohmann_json_INCLUDE_DIRS})

# Add your executable and source files
add_executable(PG2 main.cpp app.cpp)

# Link libraries
target_link_libraries(PG2
        ${OpenCV_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${GLFW_LIBRARIES}
        ${OpenGL_LIBRARIES}  # Link OpenGL
        #GL  # Explicitly link libGL
        nlohmann_json::nlohmann_json
)
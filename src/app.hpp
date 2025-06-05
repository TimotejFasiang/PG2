#pragma once

#include "imgui.h"
#include "Light.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include "Camera.hpp"
#include <nlohmann/json.hpp>
#include "assets.hpp"
#include "ShaderProgram.hpp"
#include "Model.hpp"


class App {
public:
    App();
    ~App();


    void init_assets();
    bool init();
    void updateFPS(int& frameCount, std::chrono::steady_clock::time_point& lastTime);
    void updateAnimations(float deltaTime);
    int run();
    void render();
    void checkBoundaries();
    bool isOnGround() const;
    float getTerrainHeight(float worldX, float worldZ) const;
    bool checkWallCollision(const glm::vec3& position, glm::vec3* normal = nullptr) const;
    void toggleFullscreen();
    glm::vec3 resolveWallCollision(const glm::vec3& position, const glm::vec3& velocity);
    static void printGLInfo(GLenum, const std::string&);

    // Callbacks
    static void errorCallback(int error, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void windowFocusCallback(GLFWwindow* window, int focused);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void processInput(GLFWwindow* window, float deltaTime);
    void updateProjection();

    cv::Mat mazeMap;
    std::vector<std::unique_ptr<Model>> mazeWalls;
    std::vector<std::unique_ptr<Model>> levelObjects;
    std::unique_ptr<Model> mazeFloor;
    void genLabyrinth(cv::Mat& map);

    // Maze generation methods
    void generateMaze(std::shared_ptr<ShaderProgram> shader);
    void generateTerrain();
    uchar getMapValue(int x, int y) const;

    std::unique_ptr<Model> sphereObject;

    bool isMouseVisible = false;
    bool altPressed = false;

private:


    // Physics/Collision variables
    float playerHeight = 1.62f; // Player height in world units
    float playerRadius = 0.3f; // Player collision radius
    glm::vec3 lastSafePosition; // Last safe position (no collisions)
    cv::Mat heightData;

    //light
    DirLight sun;
    std::vector<PointLight> pointLights;
    SpotLight flashlight;

    void setupLights();
    void updateLights(float deltaTime);

    Model* spinningGlassCube = nullptr;
    glm::vec3 cubeRotationSpeed = glm::vec3(50.0f, 100.0f, 80.0f);

    //cam
    Camera camera;
    glm::mat4 projection;
    float lastX = 0, lastY = 0;
    bool firstMouse = true;
    float deltaTime = 0.0f;

    // Window and rendering
    GLFWwindow* window = nullptr;
    bool vsyncOn = true;

    glm::vec3 sunWorldPosition;

    // Resources
    std::shared_ptr<ShaderProgram> main_shader;

    std::unique_ptr<Mesh> heightMapMesh;
    GLuint heightMapTexture;
    std::shared_ptr<Texture> surfaceTexture;

    void initHeightMap();
    GLuint loadHeightMapTexture(const cv::Mat& heightMap);
    std::unique_ptr<Mesh> generateHeightMap(const cv::Mat& heightMap, unsigned int stepSize);

    //Transparency
    std::vector<std::unique_ptr<Model>> transparentObjects; // For storing transparent objects
    bool antialiasingEnabled;
    int antialiasingSamples;

    struct WindowState {
        int x, y;
        int width, height;
    };

    WindowState windowedState;  // To store windowed mode position/size
    bool isFullscreen = false;  // Track fullscreen state

    // OpenGL objects
    GLuint VAO_ID = 0;
    GLuint VBO_ID = 0;

    GLuint debugTexture = 0;
    int debugTexWidth = 0;
    int debugTexHeight = 0;

    void initImGUI();
    void renderImGUI();
    void shutdownImGUI();
};
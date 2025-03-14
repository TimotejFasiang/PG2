#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <chrono>
#include <stack>
#include <random>

class App {
public:
    App();
    ~App();

    bool init();  // Initialize GLFW, GLEW, and OpenGL context
    int run();    // Main application loop
    static void printGLInfo(GLenum, const std::string&); // Print specific GL info

    // Callback functions
    static void errorCallback(int error, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
    GLFWwindow* window;  // GLFW window handle
    bool vsyncOn; // Add VSync state
};
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <stack>
#include <random>

class App {
public:
    App();
    ~App();

    bool init();  // Initialize GLFW, GLEW, and OpenGL context
    int run();    // Main application loop

private:
    GLFWwindow* window;  // GLFW window handle
};
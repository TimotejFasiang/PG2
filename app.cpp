#include "app.hpp"

App::App() : window(nullptr) {
    std::cout << "Constructed...\n";
}

App::~App() {
    // Clean up GLFW and OpenGL context
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
    std::cout << "Bye...\n";
}

bool App::init() {
    try {
        // Initialize GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // Set OpenGL version to 4.1 (or any version you prefer)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create a GLFW window
        window = glfwCreateWindow(800, 600, "OpenGL Context", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        // Make the window's context current
        glfwMakeContextCurrent(window);

        // Initialize GLEW
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("Failed to initialize GLEW");
        }

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

        std::cout << "Initialized...\n";
        return true;
    }
    catch (std::exception const& e) {
        std::cerr << "Init failed: " << e.what() << std::endl;
        throw;
    }
}

int App::run() {
    try {
        // Main application loop
        while (!glfwWindowShouldClose(window)) {
            // Clear OpenGL canvas (color buffer and depth buffer)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render or update your scene here
            // ...

            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();
        }

        std::cout << "Finished OK...\n";
        return EXIT_SUCCESS;
    }
    catch (std::exception const& e) {
        std::cerr << "App failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
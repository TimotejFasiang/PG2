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
    exit(EXIT_SUCCESS);
}

void App::printGLInfo(GLenum parameter, const std::string& parameterName) {
    if (parameter == GL_VERSION) {
        // Handle numeric version separately
        GLint majorVersion, minorVersion;
        glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

        std::cout << parameterName << ": " << majorVersion << "." << minorVersion << '\n';

        // Verify that the version is at least 4.6
        if (!(majorVersion > 4 || (majorVersion == 4 && minorVersion >= 6))) {
            std::cerr << "Error: OpenGL version is less than 4.6\n";
        }

    } else if (parameter == GL_CONTEXT_PROFILE_MASK) {
        // Handle context profile mask
        GLint profileMask;
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);

        if (profileMask & GL_CONTEXT_CORE_PROFILE_BIT) {
            std::cout << "Using CORE profile\n";
        } else if (profileMask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
            std::cout << "Using COMPATIBILITY profile\n";
        } else {
            throw std::runtime_error("Unknown OpenGL profile");
        }
    } else if (parameter == GL_CONTEXT_FLAGS) {
        // Handle context flags
        GLint contextFlags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);

        std::cout << parameterName << ":\n";
        if (contextFlags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) {
            std::cout << "  - Forward Compatible\n";
        }
        if (contextFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            std::cout << "  - Debug\n";
        }
        if (contextFlags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT) {
            std::cout << "  - Robust Access\n";
        }
        if (contextFlags & GL_CONTEXT_FLAG_NO_ERROR_BIT) {
            std::cout << "  - No Error\n";
        }
    } else {
        // Handle other string parameters
        const char* mystring = (const char*)glGetString(parameter);
        if (mystring == nullptr)
            std::cout << parameterName << ": <Unknown>\n";
        else
            std::cout << parameterName << ": " << mystring << '\n';
    }
}

bool App::init() {
    try {
        // Initialize GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // Set OpenGL version to 4.1 (or any version you prefer)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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

        // Print OpenGL information
        printGLInfo(GL_VENDOR, "OpenGL Vendor");
        printGLInfo(GL_RENDERER, "OpenGL Renderer");
        printGLInfo(GL_VERSION, "OpenGL Version");
        printGLInfo(GL_SHADING_LANGUAGE_VERSION, "GLSL Version");

        // Print context profile and flags
        printGLInfo(GL_CONTEXT_PROFILE_MASK, "OpenGL Profile");
        printGLInfo(GL_CONTEXT_FLAGS, "OpenGL Context Flags");

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
        while (!glfwWindowShouldClose(window))
        {
            if (true) {
              glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

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
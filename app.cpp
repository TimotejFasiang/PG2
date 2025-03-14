#include "app.hpp"
#include "gl_err_callback.h" // My file

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

void App::errorCallback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

void App::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Update the viewport to match the new window dimensions
    glViewport(0, 0, width, height);
    std::cout << "Window resized to: " << width << "x" << height << '\n';
}

void App::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        std::cout << "Mouse button pressed: " << button << '\n';
    } else if (action == GLFW_RELEASE) {
        std::cout << "Mouse button released: " << button << '\n';
    }
}

void App::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    // Print the cursor position (optional)
    // std::cout << "Cursor position: (" << xpos << ", " << ypos << ")\n";
}

void App::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (yoffset > 0.0) {
        std::cout << "Wheel up...\n";
    } else if (yoffset < 0.0) {
        std::cout << "Wheel down...\n";
    }
}

void App::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        // Retrieve the App instance from the window user pointer
        App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
        if (!app) {
            std::cerr << "Error: Unable to retrieve App instance from window user pointer\n";
            return;
        }

        switch (key) {
            case GLFW_KEY_ESCAPE:
                std::cout << "ESC has been pressed!\n";
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
            case GLFW_KEY_F12:
                // Toggle VSync
                    app->vsyncOn = !app->vsyncOn; // Toggle VSync state
            glfwSwapInterval(app->vsyncOn ? 1 : 0); // Update VSync setting
            std::cout << "VSync is now " << (app->vsyncOn ? "ON" : "OFF") << '\n';
            break;
            default:
                break;
        }
    }
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

        // Track if any flags are set
        bool anyFlagsSet = false;


        if (contextFlags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) {
            std::cout << "  - Forward Compatible\n";
            anyFlagsSet = true;
        }
        if (contextFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            std::cout << "  - Debug\n";
            anyFlagsSet = true;
        }
        if (contextFlags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT) {
            std::cout << "  - Robust Access\n";
            anyFlagsSet = true;
        }
        if (contextFlags & GL_CONTEXT_FLAG_NO_ERROR_BIT) {
            std::cout << "  - No Error\n";
            anyFlagsSet = true;
        }

        // If no flags are set, print "(none)"
        if (!anyFlagsSet) {
            std::cout << "  (none)\n";
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

        // Load and parse the JSON config file
        std::ifstream configFile("app_settings.json");
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open app_settings.json");
        }

        nlohmann::json config;
        configFile >> config;

        // Extract app name (optional)
        std::string appName = config.value("appname", "OpenGL Context"); // Default window title

        // Extract default resolution from the JSON file
        if (!config.contains("default_resolution") ||
            !config["default_resolution"].contains("x") ||
            !config["default_resolution"].contains("y")) {
            throw std::runtime_error("Invalid or missing default_resolution in app_settings.json");
            }

        int windowWidth = config["default_resolution"]["x"];
        int windowHeight = config["default_resolution"]["y"];

        // Validate window size
        if (windowWidth <= 0 || windowHeight <= 0) {
            throw std::runtime_error("Invalid window size in app_settings.json");
        }

        // Set GLFW error callback
        glfwSetErrorCallback(App::errorCallback);

        // Initialize GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // Set OpenGL version to 4.1 (or any version you prefer)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create a GLFW window
        window = glfwCreateWindow(windowWidth, windowHeight, appName.c_str(), nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        // Make the window's context current
        glfwMakeContextCurrent(window);

        // Initialize VSync state
        vsyncOn = true; // VSync is enabled by default
        glfwSwapInterval(vsyncOn ? 1 : 0); // Set initial VSync state

        // Set key callback and pass 'this' pointer
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, App::keyCallback);

        // Initialize GLEW
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("Failed to initialize GLEW");
        }

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

        if (GLEW_ARB_debug_output) {
		    glDebugMessageCallback(MessageCallback, 0);
		    glEnable(GL_DEBUG_OUTPUT);

            //default is asynchronous debug output, use this to simulate glGetError() functionality
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

            std::cout << "GL_DEBUG enabled." << std::endl;
	    } else {
		    std::cout << "GL_DEBUG NOT SUPPORTED!" << std::endl;
        }

        // glEnable(GL_INVALID_ENUM); // This will generate a debug message

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
        // Variables for FPS calculation
        auto lastTime = std::chrono::steady_clock::now();
        int frameCount = 0;
        double fps = 0.0;

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

            // Calculate FPS
            auto currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = currentTime - lastTime;
            frameCount++;

            // Update FPS every second
            if (elapsed.count() >= 1.0) {
                fps = frameCount / elapsed.count();
                frameCount = 0;
                lastTime = currentTime;

                // Update window title with FPS and VSync state
                std::string title = "OpenGL Context - FPS: " + std::to_string(static_cast<int>(fps)) +
                                   " | VSync: " + (vsyncOn ? "ON" : "OFF");
                glfwSetWindowTitle(window, title.c_str());
            }
        }

        std::cout << "Finished OK...\n";
        return EXIT_SUCCESS;
    }
    catch (std::exception const& e) {
        std::cerr << "App failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
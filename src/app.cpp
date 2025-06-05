#include "app.hpp"
#include <Camera.hpp>
#include "gl_err_callback.h"
#include <iostream>
#include <random>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <filesystem>
#include <stack>

App::App() : lastX(0.0f), lastY(0.0f), firstMouse(true), deltaTime(0.0f),
             window(nullptr), vsyncOn(true), VAO_ID(0),
             VBO_ID(0), debugTexture(0), debugTexWidth(0), debugTexHeight(0),
             isFullscreen(false), isMouseVisible(false), altPressed(false) {
    std::cout << "App constructed...\n";
}

App::~App() {
    // Cleanup in reverse order of creation
    shutdownImGUI();

    // Clear maze resources
    mazeWalls.clear();

    // Clear shader
    main_shader.reset();

    // GL resources
    if (VAO_ID) glDeleteVertexArrays(1, &VAO_ID);
    if (VBO_ID) glDeleteBuffers(1, &VBO_ID);
    if (debugTexture) glDeleteTextures(1, &debugTexture);

    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
    std::cout << "App destroyed...\n";
}

void App::init_assets() {
    try {
        // Shader loading with validation
        auto vertPath = "resources/basic.vert";
        auto fragPath = "resources/basic.frag";
        if (!std::filesystem::exists(vertPath)) {
            throw std::runtime_error("Vertex shader not found: " + std::string(vertPath));
        }
        if (!std::filesystem::exists(fragPath)) {
            throw std::runtime_error("Fragment shader not found: " + std::string(fragPath));
        }

        main_shader = ShaderProgram::create(vertPath, fragPath);
        if (!main_shader) {
            throw std::runtime_error("Shader program creation failed");
        }

        sphereObject = std::make_unique<Model>("resources/objects/sphere.obj", main_shader);
        sphereObject->position = glm::vec3(10.0f, 10.0f, 10.0f);
        sphereObject->setColor(glm::vec3(1.0f, 0.5f, 0.2f)); // Orange color
        sphereObject->scale = glm::vec3(1.0f);

        // Maze generation
        generateMaze(main_shader);
        initHeightMap();

        // Create transparent objects
        auto createTransparentObject = [this](const std::string& texturePath, float alpha, glm::vec3 pos) {
            auto obj = std::make_unique<Model>("resources/objects/cube.obj", main_shader);
            if (obj->setTexture(texturePath)) {
                obj->setTransparency(alpha);
                obj->position = pos;
                transparentObjects.push_back(std::move(obj));
                return true;
            }
            return false;
        };

        // Create animated objects
        auto createAnimatedObject = [this](const std::string& gifPath, float alpha, glm::vec3 pos) {
            auto obj = std::make_unique<Model>("resources/objects/cube.obj", main_shader);
            if (obj->setAnimatedTexture(gifPath)) {
                obj->setTransparency(alpha);
                obj->position = pos;
                transparentObjects.push_back(std::move(obj));
                return true;
            }
            return false;
        };

        // Transparent objects
        createTransparentObject("resources/textures/glass.png", 1.0f, glm::vec3(9.501f, 0.501f, 4.5f));
        createTransparentObject("resources/textures/glass.png", 1.0f, glm::vec3(9.501f, 1.501f, 4.5f));
        bool spinningCube = createTransparentObject("resources/textures/glass.png", 1.0f, glm::vec3(9.501f, 4.0f, 4.5f));
        spinningGlassCube = transparentObjects.back().get();

        // Animated objects
        createAnimatedObject("resources/textures/water.gif", 0.75f, glm::vec3(9.501f, 0.501f, 2.5f));
        createAnimatedObject("resources/textures/lava.gif", 1.0f, glm::vec3(9.501f, 0.501f, 6.5f));

    } catch (const std::exception& e) {
        std::cerr << "Asset initialization failed: " << e.what() << std::endl;
        throw;
    }
}

bool App::init() {
    try {
        // Load config
        std::ifstream configFile("app_settings.json");
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open app_settings.json");
        }

        nlohmann::json config;
        configFile >> config;

        // Window setup
        int width = config["default_resolution"]["x"];
        int height = config["default_resolution"]["y"];
        std::string title = config.value("appname", "OpenGL Maze");

        // GLFW init
        glfwSetErrorCallback(App::errorCallback);
        if (!glfwInit()) {
            throw std::runtime_error("GLFW initialization failed");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        // Load AA settings first
        antialiasingEnabled = config["antialiasing"]["enabled"];
        antialiasingSamples = config["antialiasing"]["samples"];

        // Set window hints for AA if enabled
        // if (antialiasingEnabled) {
            glfwWindowHint(GLFW_SAMPLES, antialiasingSamples);
        // }

        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("Window creation failed");
        }
        glfwGetWindowPos(window, &windowedState.x, &windowedState.y);
        glfwGetWindowSize(window, &windowedState.width, &windowedState.height);
        isFullscreen = false;
        glfwSetWindowFocusCallback(window, windowFocusCallback);


        glfwMakeContextCurrent(window);
        glfwSwapInterval(vsyncOn ? 1 : 0);

        // GLEW init
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("GLEW initialization failed");
        }

        // Print OpenGL context info
        std::cout << "\n--- OpenGL Context Information ---\n";
        printGLInfo(GL_VERSION, "OpenGL Version");
        printGLInfo(GL_VENDOR, "Vendor");
        printGLInfo(GL_RENDERER, "Renderer");
        printGLInfo(GL_SHADING_LANGUAGE_VERSION, "GLSL Version");
        printGLInfo(GL_CONTEXT_PROFILE_MASK, "Context Profile");
        printGLInfo(GL_CONTEXT_FLAGS, "Context Flags");

        // OpenGL config
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        if (GLEW_ARB_debug_output) {
            glDebugMessageCallback(MessageCallback, 0);
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        }

        // Callbacks
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, App::keyCallback);
        glfwSetFramebufferSizeCallback(window, App::framebufferSizeCallback);
        glfwSetCursorPosCallback(window, mouseCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        isMouseVisible = false;

        // Initialize systems
        initImGUI();
        init_assets();

        // Initial camera setup
        updateProjection();
        camera.Position = glm::vec3(15.0f, playerHeight, 5.0f); // Start at ground level
        camera.Yaw = 180.0f;
        camera.updateCameraVectors();

        // Set initial safe position
        lastSafePosition = camera.Position;

        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_LEQUAL); // Helps with transparency sorting

        setupLights();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Initialization failed: " << e.what() << std::endl;
        return false;
    }
}

int App::run() {
    float lastFrame = 0.0f;
    auto lastTime = std::chrono::steady_clock::now();
    int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        // Timing calculations
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        updateLights(deltaTime);

        // Update animations
        updateAnimations(deltaTime);

        processInput(window, deltaTime);
        glDisable(GL_CULL_FACE);
        render();
        glEnable(GL_CULL_FACE);
        updateFPS(frameCount, lastTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    return EXIT_SUCCESS;
}

void App::render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable multisampling if AA is enabled
    if (antialiasingEnabled) {
        glEnable(GL_MULTISAMPLE);
        // Enable sample shading for better quality
        glEnable(GL_SAMPLE_SHADING);
        glMinSampleShading(1.0f);
    } else {
        glDisable(GL_MULTISAMPLE);
        glDisable(GL_SAMPLE_SHADING);
    }

    main_shader->activate();
    main_shader->setUniform("dirLight.direction", sun.direction);
    main_shader->setUniform("dirLight.ambient", sun.ambient);
    main_shader->setUniform("dirLight.diffuse", sun.diffuse);
    main_shader->setUniform("dirLight.specular", sun.specular);

    for (int i = 0; i < pointLights.size(); i++) {
        std::string prefix = "pointLights[" + std::to_string(i) + "].";
        main_shader->setUniform(prefix + "position", pointLights[i].position);
        main_shader->setUniform(prefix + "ambient", pointLights[i].ambient);
        main_shader->setUniform(prefix + "diffuse", pointLights[i].diffuse);
        main_shader->setUniform(prefix + "specular", pointLights[i].specular);
        main_shader->setUniform(prefix + "constant", pointLights[i].constant);
        main_shader->setUniform(prefix + "linear", pointLights[i].linear);
        main_shader->setUniform(prefix + "quadratic", pointLights[i].quadratic);
    }

    main_shader->setUniform("spotLight.position", flashlight.position);
    main_shader->setUniform("spotLight.direction", flashlight.direction);
    main_shader->setUniform("spotLight.cutOff", flashlight.cutOff);
    main_shader->setUniform("spotLight.outerCutOff", flashlight.outerCutOff);
    main_shader->setUniform("spotLight.ambient", flashlight.ambient);
    main_shader->setUniform("spotLight.diffuse", flashlight.diffuse);
    main_shader->setUniform("spotLight.specular", flashlight.specular);
    main_shader->setUniform("spotLight.constant", flashlight.constant);
    main_shader->setUniform("spotLight.linear", flashlight.linear);
    main_shader->setUniform("spotLight.quadratic", flashlight.quadratic);

    main_shader->setUniform("projection", projection);
    main_shader->setUniform("view", camera.GetViewMatrix());
    main_shader->setUniform("viewPos", camera.Position);

    // Render heightmap with moon surface texture
    if (heightMapMesh) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        main_shader->setUniform("model", model);
        main_shader->setUniform("useTexture", 1); // Always use texture for heightmap

        if (surfaceTexture) {
            surfaceTexture->bind(GL_TEXTURE0);
            main_shader->setUniform("diffuseTexture", 0);
        }

        // Set material properties for heightmap
        main_shader->setUniform("objectColor", glm::vec3(1.0f)); // White base color
        main_shader->setUniform("alpha", 1.0f); // Fully opaque

        heightMapMesh->draw();
    }

    // First pass: draw all completely opaque objects
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    sphereObject->position = sunWorldPosition;
    sphereObject->scale = glm::vec3(5.0f);
    glDisable(GL_DEPTH_TEST);
    sphereObject->draw();
    glEnable(GL_DEPTH_TEST);

    for (const auto& wall : mazeWalls) {
        if (!wall->transparent) {
            wall->draw();
        }
    }

    // Separate objects into opaque and transparent lists
    std::vector<Model*> opaqueObjects;
    std::vector<Model*> transparentObjects;

    for (auto& obj : this->transparentObjects) {
        if (obj->hasTransparency()) {
            transparentObjects.push_back(obj.get());
        } else {
            opaqueObjects.push_back(obj.get());
        }
    }

    // Draw nearly-opaque objects without blending
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    for (auto obj : opaqueObjects) {
        obj->draw();
    }

    // Sort and draw truly transparent objects
    std::sort(transparentObjects.begin(), transparentObjects.end(),
        [this](const Model* a, const Model* b) {
            return glm::distance(camera.Position, a->position) >
                   glm::distance(camera.Position, b->position);
        });

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (auto obj : transparentObjects) {
        obj->draw();
    }

    // Restore state
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    renderImGUI();
}


void App::setupLights() {
    // Directional light (sun)
    sun = {
        .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        .ambient = glm::vec3(0.1f),
        .diffuse = glm::vec3(0.5f),
        .specular = glm::vec3(0.5f)
    };

    // Point lights
    pointLights = {
        { // Red light
            .position = glm::vec3(9.5f, 0.5f, 6.5f),
            .ambient = glm::vec3(0.05f, 0.0f, 0.0f),
            .diffuse = glm::vec3(0.8f, 0.0f, 0.0f),
            .specular = glm::vec3(1.0f, 0.0f, 0.0f),
            .constant = 1.0f,
            .linear = 0.09f,
            .quadratic = 0.032f
        },
        { // Green light
            .position = glm::vec3(100.0f, 100.0f, 100.0f),
            .ambient = glm::vec3(0.0f, 0.1f, 0.0f),
            .diffuse = glm::vec3(0.0f, 0.1f, 0.0f),
            .specular = glm::vec3(0.0f, 0.1f, 0.0f),
            .constant = 1.0f,
            .linear = 0.09f,
            .quadratic = 0.032f
        },
        { // Blue light
            .position = glm::vec3(9.5f, 0.5f, 2.5f),
            .ambient = glm::vec3(0.0f, 0.0f, 0.05f),
            .diffuse = glm::vec3(0.0f, 0.0f, 0.8f),
            .specular = glm::vec3(0.0f, 0.0f, 1.0f),
            .constant = 1.0f,
            .linear = 0.09f,
            .quadratic = 0.032f
        }
    };

    // Flashlight (attached to camera)
    flashlight = {
        .position = camera.Position,
        .direction = camera.Front,
        .cutOff = glm::cos(glm::radians(12.5f)),
        .outerCutOff = glm::cos(glm::radians(17.5f)),
        .ambient = glm::vec3(0.0f),
        .diffuse = glm::vec3(1.0f),
        .specular = glm::vec3(1.0f),
        .constant = 1.0f,
        .linear = 0.09f,
        .quadratic = 0.032f
    };
}

void App::updateLights(float deltaTime) {
    // Simple day/night cycle
    static float sunAngle = 0.0f;
    sunAngle += deltaTime * 0.1f;

    // Sun direction (pointing TOWARD the scene)
    sun.direction = glm::normalize(glm::vec3(
        cos(sunAngle),
        sin(sunAngle) * 0.5f - 0.7f,  // Keeps sun mostly above horizon
        sin(sunAngle)
    ));

    // Sun visual position (pointing AWAY from the scene)
    // Far away in the opposite direction
    sunWorldPosition = -sun.direction * 400.0f;

    // Update flashlight to follow camera
    flashlight.position = camera.Position;
    flashlight.direction = camera.Front;

    // Make point lights pulse
    static float pulse = 0.0f;
    pulse += deltaTime;
    pointLights[0].diffuse.r = 0.8f + sin(pulse) * 0.2f;
    pointLights[1].diffuse.g = 0.8f + cos(pulse*0.7f) * 0.2f;
    pointLights[2].diffuse.b = 0.8f + sin(pulse*1.3f) * 0.2f;
}


void App::updateAnimations(float deltaTime) {
    for (auto& obj : transparentObjects) {
        obj->update(deltaTime);
    }
    if (spinningGlassCube) {
        spinningGlassCube->rotation += cubeRotationSpeed * deltaTime;

        // Keep rotations within 0-360 degrees
        if (spinningGlassCube->rotation.x >= 360.0f) spinningGlassCube->rotation.x -= 360.0f;
        if (spinningGlassCube->rotation.y >= 360.0f) spinningGlassCube->rotation.y -= 360.0f;
        if (spinningGlassCube->rotation.z >= 360.0f) spinningGlassCube->rotation.z -= 360.0f;
    }
}

void App::updateFPS(int& frameCount, std::chrono::steady_clock::time_point& lastTime) {
    frameCount++;
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime);

    if (elapsed.count() >= 1) {
        double fps = frameCount / elapsed.count();
        std::string title = "Maze Renderer - FPS: " + std::to_string(static_cast<int>(fps));
        glfwSetWindowTitle(window, title.c_str());
        frameCount = 0;
        lastTime = currentTime;
    }
}

uchar App::getMapValue(int x, int y) const {
    x = std::clamp(x, 0, mazeMap.cols - 1);
    y = std::clamp(y, 0, mazeMap.rows - 1);
    return mazeMap.at<uchar>(y, x);
}

void App::initHeightMap() {
    // Load heightmap image
    cv::Mat heightMap = cv::imread("resources/textures/heightmap_3_inverted.png", cv::IMREAD_GRAYSCALE);
    if (heightMap.empty()) {
        throw std::runtime_error("Failed to load heightmap texture");
    }

    // Store the height data for CPU access
    heightData = heightMap.clone();

    // Generate mesh and height data texture
    heightMapMesh = generateHeightMap(heightMap, 2);
    heightMapTexture = loadHeightMapTexture(heightMap);

    // Load surface texture
    surfaceTexture = Texture::create("resources/textures/moon_surface_tiled3.png");
    if (!surfaceTexture || !surfaceTexture->valid()) {
        throw std::runtime_error("Failed to load moon surface texture");
    }
}

GLuint App::loadHeightMapTexture(const cv::Mat& heightMap) {
    GLuint textureID;
    glCreateTextures(GL_TEXTURE_2D, 1, &textureID);

    // Configure texture
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Allocate storage and upload data
    glTextureStorage2D(textureID, 1, GL_R8, heightMap.cols, heightMap.rows);
    glTextureSubImage2D(textureID, 0, 0, 0, heightMap.cols, heightMap.rows,
                      GL_RED, GL_UNSIGNED_BYTE, heightMap.data);

    return textureID;
}

std::unique_ptr<Mesh> App::generateHeightMap(const cv::Mat& heightMap, unsigned int stepSize) {
    std::vector<vertex> vertices;
    std::vector<GLuint> indices;
    GLuint index = 0;

    const float heightScale = 1.0f / 255.0f * 2;
    const float worldScale = 0.2f;
    const float heightMapBaseY = 0.0f;

    // Texture tiling factor
    const float textureTileFactor = 15.0f;

    // Calculate offset to center the heightmap (XZ only)
    float xOffset = -heightMap.cols * worldScale * 0.5f;
    float zOffset = -heightMap.rows * worldScale * 0.5f;

    for (unsigned int z = 0; z < heightMap.rows - stepSize; z += stepSize) {
        for (unsigned int x = 0; x < heightMap.cols - stepSize; x += stepSize) {
            // Get height values (0-1 range)
            float h0 = heightMap.at<uchar>(z, x) * heightScale;
            float h1 = heightMap.at<uchar>(z, x + stepSize) * heightScale;
            float h2 = heightMap.at<uchar>(z + stepSize, x + stepSize) * heightScale;
            float h3 = heightMap.at<uchar>(z + stepSize, x) * heightScale;

            // Vertex positions (Y coordinate uses explicit base + scaled height)
            glm::vec3 p0(
                x * worldScale + xOffset,
                heightMapBaseY + h0 * 10.0f,
                z * worldScale + zOffset
            );
            glm::vec3 p1(
                (x + stepSize) * worldScale + xOffset,
                heightMapBaseY + h1 * 10.0f,
                z * worldScale + zOffset
            );
            glm::vec3 p2(
                (x + stepSize) * worldScale + xOffset,
                heightMapBaseY + h2 * 10.0f,
                (z + stepSize) * worldScale + zOffset
            );
            glm::vec3 p3(
                x * worldScale + xOffset,
                heightMapBaseY + h3 * 10.0f,
                (z + stepSize) * worldScale + zOffset
            );

            // Calculate normal
            glm::vec3 normal = glm::normalize(glm::cross(p2 - p0, p1 - p0));

            // Modified texture coordinates - multiplied by tile factor
            glm::vec2 t0(
                static_cast<float>(x) / heightMap.cols * textureTileFactor,
                static_cast<float>(z) / heightMap.rows * textureTileFactor
            );
            glm::vec2 t1(
                static_cast<float>(x + stepSize) / heightMap.cols * textureTileFactor,
                static_cast<float>(z) / heightMap.rows * textureTileFactor
            );
            glm::vec2 t2(
                static_cast<float>(x + stepSize) / heightMap.cols * textureTileFactor,
                static_cast<float>(z + stepSize) / heightMap.rows * textureTileFactor
            );
            glm::vec2 t3(
                static_cast<float>(x) / heightMap.cols * textureTileFactor,
                static_cast<float>(z + stepSize) / heightMap.rows * textureTileFactor
            );

            // Add vertices
            vertices.emplace_back(p0, normal, t0);
            vertices.emplace_back(p1, normal, t1);
            vertices.emplace_back(p2, normal, t2);
            vertices.emplace_back(p3, normal, t3);

            // Add indices (two triangles per quad)
            indices.push_back(index);
            indices.push_back(index + 1);
            indices.push_back(index + 2);
            indices.push_back(index);
            indices.push_back(index + 2);
            indices.push_back(index + 3);

            index += 4;
        }
    }

    return std::make_unique<Mesh>(GL_TRIANGLES, main_shader, vertices, indices);
}

void App::generateMaze(std::shared_ptr<ShaderProgram> shader) {
    const int mazeWidth = 19;
    const int mazeHeight = 19;
    mazeMap = cv::Mat(mazeHeight, mazeWidth, CV_8U);
    genLabyrinth(mazeMap);

    const float worldScale = 1.0f;
    const float mazeElevation = 0.5f;

    mazeWalls.clear();

    // Render all cells, including outer walls
    for (int y = 0; y < mazeMap.rows; y++) {
        for (int x = 0; x < mazeMap.cols; x++) {
            uchar cell = getMapValue(x, y);

            // Only render walls, skip paths and openings
            if (cell == '#') {
                // Skip rendering the entrance and exit openings
                bool isEntrance = (x == 0 && y == 1);
                bool isExit = (x == mazeMap.cols-1 && y == mazeMap.rows-2);

                if (!isEntrance && !isExit) {
                    auto wall = std::make_unique<Model>("resources/objects/cube.obj", shader);
                    if (wall->setTexture("resources/textures/box.jpg")) {
                        wall->position = glm::vec3(
                            (x - mazeWidth/2.0f) * worldScale,
                            mazeElevation,
                            (y - mazeHeight/2.0f) * worldScale
                        );
                        wall->scale = glm::vec3(worldScale);
                        mazeWalls.push_back(std::move(wall));
                    }
                }
            }
        }
    }
}

void App::genLabyrinth(cv::Mat& map) {
    // Initialize with all walls
    map.setTo('#');

    std::random_device rd;
    std::mt19937 gen(rd());

    // Start position (must be odd coordinates)
    int startX = 1;
    int startY = 1;

    std::stack<cv::Point> stack;
    stack.push(cv::Point(startX, startY));
    map.at<uchar>(startY, startX) = '.';

    // Directions: up, right, down, left
    const int dx[] = {0, 1, 0, -1};
    const int dy[] = {-1, 0, 1, 0};

    while (!stack.empty()) {
        cv::Point current = stack.top();
        std::vector<int> directions = {0, 1, 2, 3};
        std::shuffle(directions.begin(), directions.end(), gen);

        bool found = false;
        for (int dir : directions) {
            int nx = current.x + dx[dir] * 2;
            int ny = current.y + dy[dir] * 2;

            if (nx >= 1 && nx < map.cols-1 && ny >= 1 && ny < map.rows-1 &&
                map.at<uchar>(ny, nx) == '#') {
                // Carve path
                map.at<uchar>(current.y + dy[dir], current.x + dx[dir]) = '.';
                map.at<uchar>(ny, nx) = '.';
                stack.push(cv::Point(nx, ny));
                found = true;
                break;
                }
        }

        if (!found) {
            stack.pop();
        }
    }

    // Create entrance and exit openings while maintaining outer walls
    // Entrance (left side)
    map.at<uchar>(1, 0) = '.';
    // Exit (right side)
    map.at<uchar>(map.rows-2, map.cols-1) = '.';

    // Mark start and exit positions
    map.at<uchar>(1, 1) = 's';
    map.at<uchar>(map.rows-2, map.cols-2) = 'e';
}

void App::updateProjection() {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    projection = glm::perspective(glm::radians(camera.Zoom),
                                static_cast<float>(width)/static_cast<float>(height),
                                0.1f, 1000.0f);
}

void App::initImGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // Load debug texture
    cv::Mat texImage = cv::imread("resources/textures/debug_tex.png");
    if (!texImage.empty()) {
        cv::cvtColor(texImage, texImage, cv::COLOR_BGR2RGB);

        glCreateTextures(GL_TEXTURE_2D, 1, &debugTexture);
        glTextureStorage2D(debugTexture, 1, GL_RGB8, texImage.cols, texImage.rows);
        glTextureSubImage2D(debugTexture, 0, 0, 0, texImage.cols, texImage.rows,
                          GL_RGB, GL_UNSIGNED_BYTE, texImage.data);
        glGenerateTextureMipmap(debugTexture);

        debugTexWidth = texImage.cols;
        debugTexHeight = texImage.rows;
    } else {
        std::cerr << "Failed to load debug texture\n";
    }
}

void App::renderImGUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug Info");
    ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)",
               camera.Position.x, camera.Position.y, camera.Position.z);
    ImGui::Text("Facing: (%.1f, %.1f, %.1f)",
               camera.Front.x, camera.Front.y, camera.Front.z);
    ImGui::Text("Maze Size: %dx%d", mazeMap.cols, mazeMap.rows);
    ImGui::Text("Walls: %zu", mazeWalls.size());
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::shutdownImGUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

///////////////////////////////////// Static callback functions ///////////////////////////////////////////////////////
void App::errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void App::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app) {
        // Only update windowed state if in windowed mode
        if (!app->isFullscreen) {
            glfwGetWindowPos(window, &app->windowedState.x, &app->windowedState.y);
            app->windowedState.width = width;
            app->windowedState.height = height;
        }
        app->updateProjection();
    }
}

void App::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        std::cout << "Mouse button pressed: " << button << '\n';
    } else if (action == GLFW_RELEASE) {
        std::cout << "Mouse button released: " << button << '\n';
    }
}

void App::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app || app->isMouseVisible) return;  // Only process when mouse is invisible

    if (app->firstMouse) {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }

    float xoffset = xpos - app->lastX;
    float yoffset = app->lastY - ypos;

    app->lastX = xpos;
    app->lastY = ypos;

    app->camera.ProcessMouseMovement(xoffset, yoffset);
}

void App::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->camera.ProcessMouseScroll(yoffset);
        app->updateProjection();
    }
}

void App::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    // Handle Alt+Tab to show mouse
    if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT) {
        if (action == GLFW_PRESS) {
            app->altPressed = true;
        } else if (action == GLFW_RELEASE) {
            app->altPressed = false;
        }
    }

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS && app->altPressed) {
        app->isMouseVisible = !app->isMouseVisible;
        glfwSetInputMode(window, GLFW_CURSOR,
            app->isMouseVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

        // Reset mouse position to avoid sudden jumps
        if (!app->isMouseVisible) {
            app->firstMouse = true;
        }
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_F12:
            app->vsyncOn = !app->vsyncOn;
            glfwSwapInterval(app->vsyncOn ? 1 : 0);
            std::cout << "VSync " << (app->vsyncOn ? "enabled" : "disabled") << "\n";
            break;
        case GLFW_KEY_R:
            app->generateMaze(app->main_shader);
            std::cout << "Regenerated maze\n";
            break;
        case GLFW_KEY_F1:
            app->antialiasingEnabled = !app->antialiasingEnabled;
            std::cout << "Antialiasing " << (app->antialiasingEnabled ? "enabled" : "disabled") << "\n";
            if (app->antialiasingEnabled) {
                if (app->antialiasingSamples <= 1) {
                    std::cerr << "Warning: Antialiasing enabled but samples <= 1, disabling AA\n";
                    app->antialiasingEnabled = false;
                } else if (app->antialiasingSamples > 8) {
                    std::cerr << "Warning: Antialiasing samples > 8, clamping to 8\n";
                    app->antialiasingSamples = 8;
                }
            }
            break;
        case GLFW_KEY_F11:  // Add this case for fullscreen toggle
            app->toggleFullscreen();
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

/////////////////// Physics /////////////////////////////
void App::processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Movement direction (XZ plane only)
    glm::vec3 moveDir(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveDir += glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveDir -= glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveDir -= glm::normalize(glm::vec3(camera.Right.x, 0.0f, camera.Right.z));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveDir += glm::normalize(glm::vec3(camera.Right.x, 0.0f, camera.Right.z));

    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
        glm::vec3 velocity = moveDir * 2.5f * deltaTime;

        glm::vec3 newPosition = camera.Position + velocity;
        if (!checkWallCollision(newPosition, {})) {
            camera.Position = newPosition;
        } else {
            // Get collision normal
            glm::vec3 collisionNormal(0.0f);
            glm::vec3 testPos = camera.Position + glm::vec3(velocity.x, 0, 0);
            if (checkWallCollision(testPos, {})) collisionNormal.x = 1.0f;

            testPos = camera.Position + glm::vec3(0, 0, velocity.z);
            if (checkWallCollision(testPos, {})) collisionNormal.z = 1.0f;

            // Calculate slide direction
            if (glm::length(collisionNormal) > 0.0f) {
                collisionNormal = glm::normalize(collisionNormal);
                glm::vec3 slideVelocity = velocity - collisionNormal * glm::dot(velocity, collisionNormal);

                // Slide movement
                newPosition = camera.Position + slideVelocity;
                if (!checkWallCollision(newPosition, {})) {
                    camera.Position = newPosition;
                }
            }
        }

        camera.Position.y = playerHeight;
        lastSafePosition = camera.Position;
    }
}

bool App::checkWallCollision(const glm::vec3& position, glm::vec3* normal) const {
    const float wallHalfSize = 0.5f;
    const float playerRadius = 0.3f;
    const float maxDist = playerRadius + wallHalfSize;
    bool collision = false;

    auto checkObject = [&](const glm::vec3& objPos) -> bool {
        if (glm::distance(glm::vec2(position.x, position.z),
                         glm::vec2(objPos.x, objPos.z)) > maxDist * 1.5f) {
            return false;
                         }

        // Precise collision check
        float overlapX = maxDist - abs(position.x - objPos.x);
        float overlapZ = maxDist - abs(position.z - objPos.z);

        if (overlapX > 0 && overlapZ > 0) {
            if (normal) {
                if (overlapX < overlapZ) {
                    *normal = glm::vec3((position.x < objPos.x) ? -1.0f : 1.0f, 0.0f, 0.0f);
                } else {
                    *normal = glm::vec3(0.0f, 0.0f, (position.z < objPos.z) ? -1.0f : 1.0f);
                }
            }
            return true;
        }
        return false;
    };

    // Check maze walls
    for (const auto& wall : mazeWalls) {
        if (checkObject(wall->position)) {
            return true;
        }
    }

    // Check transparent objects - only those that should block movement
    for (const auto& obj : transparentObjects) {
        // Skip water and lava by checking animatedTextures
        if (obj->hasAnimatedTexture()) {
            continue; // Skip water and lava
        }

        if (checkObject(obj->position)) {
            return true;
        }
    }

    return false;
}

void App::toggleFullscreen() {
    if (isFullscreen) {
        // Switch to windowed mode
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // Ensure the window will be visible on screen
        int x = windowedState.x;
        int y = windowedState.y;
        int width = windowedState.width;
        int height = windowedState.height;

        // Basic bounds checking
        if (x + width < 50 || y + height < 50) {
            x = 100;
            y = 100;
        }

        glfwSetWindowMonitor(window, nullptr, x, y, width, height, GLFW_DONT_CARE);
        isFullscreen = false;
    } else {
        // Store current window state before going fullscreen
        glfwGetWindowPos(window, &windowedState.x, &windowedState.y);
        glfwGetWindowSize(window, &windowedState.width, &windowedState.height);

        // Switch to fullscreen mode
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor,
                            0, 0,
                            mode->width, mode->height,
                            mode->refreshRate);
        isFullscreen = true;
    }

    updateProjection();
}

void App::windowFocusCallback(GLFWwindow* window, int focused) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app) {
        if (focused) {
            // Window gained focus - restore previous cursor state
            glfwSetInputMode(window, GLFW_CURSOR,
                app->isMouseVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

            // Reset mouse position to avoid sudden jumps
            if (!app->isMouseVisible) {
                app->firstMouse = true;
            }
        } else {
            // Window lost focus - show cursor
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}
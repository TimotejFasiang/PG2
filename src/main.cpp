#include "app.hpp"

int main(int argc, char** argv) {
    App app;
    try {
        if (app.init()) {
            return app.run();
        }
    } catch (const std::exception& e) {
        std::cerr << "App failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
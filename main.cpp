#include "app.hpp"

int main(int argc, char** argv) {
    App app;

    try {
        if (app.init()) {
            return app.run();
        }
    }
    catch (std::exception const& e) {
        std::cerr << "App failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
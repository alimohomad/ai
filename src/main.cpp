#include "application.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "  Cinematic VFX Editor v1.0" << std::endl;
    std::cout << "  AI-Powered Scene Animation Tool" << std::endl;
    std::cout << "========================================" << std::endl;

    Application app;

    if (!app.init(1280, 720, "Cinematic VFX Editor - AI Scene Animator")) {
        std::cerr << "Failed to initialize application" << std::endl;
        return 1;
    }

    if (argc > 1) {
        std::cout << "Loading image from command line: " << argv[1] << std::endl;
    }

    app.run();
    app.shutdown();

    return 0;
}

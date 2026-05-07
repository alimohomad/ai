#pragma once
#include "gl_utils.h"
#include "scene.h"
#include "vfx_system.h"
#include "post_process.h"
#include "camera.h"
#include "editor.h"
#include "video_export.h"
#include <GLFW/glfw3.h>

class Application {
public:
    bool init(int width = 1280, int height = 720, const char* title = "Cinematic VFX Editor");
    void run();
    void shutdown();

private:
    GLFWwindow* window = nullptr;
    int windowWidth = 1280, windowHeight = 720;

    Scene scene;
    VFXSystem vfxSystem;
    PostProcessor postProcessor;
    Camera camera;
    Editor editor;
    VideoExporter exporter;

    PostProcessSettings ppSettings;

    Shader quadShader;
    QuadRenderer quadRenderer;
    Framebuffer sceneFBO;
    int sceneFBOWidth = 800, sceneFBOHeight = 600;

    float lastFrameTime = 0.0f;
    float deltaTime = 0.0f;

    void initImGui();
    void updateScene(float dt);
    void renderSceneToFBO();
    void handleExport();
    void processInput();

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

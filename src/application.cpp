#include "application.h"
#include "shaders.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

bool Application::init(int width, int height, const char* title) {
    windowWidth = width;
    windowHeight = height;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initImGui();

    quadShader.init(Shaders::SCREEN_QUAD_VERT, Shaders::SCREEN_QUAD_FRAG);
    quadRenderer.init();
    vfxSystem.init();

    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    sceneFBOWidth = 1280;
    sceneFBOHeight = 720;
    sceneFBO.create(sceneFBOWidth, sceneFBOHeight);
    postProcessor.init(sceneFBOWidth, sceneFBOHeight);

    editor.init();

    std::cout << "Cinematic VFX Editor initialized successfully" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    return true;
}

void Application::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.FontGlobalScale = 1.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Application::run() {
    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        if (deltaTime > 0.1f) deltaTime = 0.016f;

        glfwPollEvents();
        processInput();

        // Handle image loading requests
        if (editor.wantsLoadImage()) {
            std::string path = editor.getRequestedImagePath();
            editor.clearLoadRequest();

            scene.destroy();
            if (scene.loadImage(path)) {
                sceneFBOWidth = scene.imageWidth;
                sceneFBOHeight = scene.imageHeight;
                sceneFBO.destroy();
                sceneFBO.create(sceneFBOWidth, sceneFBOHeight);
                postProcessor.resize(sceneFBOWidth, sceneFBOHeight);
                editor.state.statusMessage = "Image loaded: " + path;
                std::cout << "Loaded image: " << path << " (" << scene.imageWidth << "x" << scene.imageHeight << ")" << std::endl;
            } else {
                editor.state.statusMessage = "Failed to load: " + path;
            }
        }

        // Handle export requests
        if (editor.wantsExport()) {
            editor.clearExportRequest();
            handleExport();
        }

        // Update systems
        if (editor.state.isPlaying) {
            camera.update(deltaTime);
            vfxSystem.update(deltaTime, static_cast<float>(sceneFBOWidth), static_cast<float>(sceneFBOHeight));
            editor.state.timelinePosition += deltaTime;
            if (editor.state.timelinePosition > editor.state.timelineDuration) {
                editor.state.timelinePosition = 0.0f;
            }
        }

        // Render scene to FBO
        renderSceneToFBO();

        // Post-process
        ppSettings.lightningFlash = vfxSystem.getLightningFlash();
        postProcessor.process(sceneFBO.colorTexture, ppSettings);

        // Render ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        editor.render(scene, vfxSystem, camera, ppSettings,
                     static_cast<float>(sceneFBOWidth), static_cast<float>(sceneFBOHeight),
                     postProcessor.getOutputTexture());

        ImGui::Render();

        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}

void Application::renderSceneToFBO() {
    sceneFBO.bind();
    glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 projection = camera.getProjectionMatrix(
        static_cast<float>(sceneFBOWidth), static_cast<float>(sceneFBOHeight));
    glm::mat4 view = camera.getViewMatrix(
        static_cast<float>(sceneFBOWidth), static_cast<float>(sceneFBOHeight));

    if (scene.loaded) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (editor.state.showLayers && !scene.layers.empty()) {
            // Render separated layers with parallax
            for (int i = static_cast<int>(scene.layers.size()) - 1; i >= 0; i--) {
                auto& layer = scene.layers[i];
                if (!layer.visible) continue;

                glm::vec2 parallax = camera.getParallaxOffset(layer.depth);
                glm::mat4 model(1.0f);
                model = glm::translate(model, glm::vec3(parallax.x, parallax.y, 0.0f));
                model = glm::scale(model, glm::vec3(
                    static_cast<float>(sceneFBOWidth) * 0.5f,
                    static_cast<float>(sceneFBOHeight) * 0.5f, 1.0f));
                model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));

                quadShader.use();
                quadShader.setMat4("uProjection", projection * view);
                quadShader.setMat4("uModel", model);
                quadShader.setFloat("uAlpha", 1.0f);
                layer.texture.bind(0);
                quadShader.setInt("uTexture", 0);
                quadRenderer.renderQuad();
            }
        } else if (editor.state.showDepthMap && scene.depthMap.texture.id != 0) {
            // Show depth map
            glm::mat4 model(1.0f);
            model = glm::scale(model, glm::vec3(
                static_cast<float>(sceneFBOWidth) * 0.5f,
                static_cast<float>(sceneFBOHeight) * 0.5f, 1.0f));
            model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));

            quadShader.use();
            quadShader.setMat4("uProjection", projection * view);
            quadShader.setMat4("uModel", model);
            quadShader.setFloat("uAlpha", 1.0f);
            scene.depthMap.texture.bind(0);
            quadShader.setInt("uTexture", 0);
            quadRenderer.renderQuad();
        } else {
            // Render main image
            glm::mat4 model(1.0f);
            model = glm::scale(model, glm::vec3(
                static_cast<float>(sceneFBOWidth) * 0.5f,
                static_cast<float>(sceneFBOHeight) * 0.5f, 1.0f));
            model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));

            quadShader.use();
            quadShader.setMat4("uProjection", projection * view);
            quadShader.setMat4("uModel", model);
            quadShader.setFloat("uAlpha", 1.0f);
            scene.mainImage.bind(0);
            quadShader.setInt("uTexture", 0);
            quadRenderer.renderQuad();
        }
    }

    // Render VFX particles
    glm::mat4 particleProjection = projection * view;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    vfxSystem.render(particleProjection);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    sceneFBO.unbind();
}

void Application::handleExport() {
    if (exporter.isExporting()) return;

    int w = editor.state.exportWidth;
    int h = editor.state.exportHeight;
    int fps = editor.state.exportFPS;
    float duration = editor.state.exportDuration;
    int totalFrames = static_cast<int>(duration * fps);

    if (!exporter.start(editor.state.exportPath, w, h, fps)) {
        editor.state.statusMessage = "Export failed to start";
        return;
    }

    editor.state.exporting = true;

    Framebuffer exportFBO;
    exportFBO.create(w, h);

    PostProcessor exportPP;
    exportPP.init(w, h);

    Framebuffer exportSceneFBO;
    exportSceneFBO.create(w, h);

    float dt = 1.0f / fps;
    Camera exportCamera = camera;

    std::vector<unsigned char> pixels(w * h * 3);

    for (int frame = 0; frame < totalFrames; frame++) {
        exportCamera.update(dt);
        vfxSystem.update(dt, static_cast<float>(w), static_cast<float>(h));

        // Render to export FBO
        exportSceneFBO.bind();
        glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 proj = exportCamera.getProjectionMatrix(static_cast<float>(w), static_cast<float>(h));
        glm::mat4 view = exportCamera.getViewMatrix(static_cast<float>(w), static_cast<float>(h));

        if (scene.loaded) {
            glm::mat4 model(1.0f);
            model = glm::scale(model, glm::vec3(w * 0.5f, h * 0.5f, 1.0f));
            model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));

            quadShader.use();
            quadShader.setMat4("uProjection", proj * view);
            quadShader.setMat4("uModel", model);
            quadShader.setFloat("uAlpha", 1.0f);
            scene.mainImage.bind(0);
            quadShader.setInt("uTexture", 0);
            quadRenderer.renderQuad();
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        vfxSystem.render(proj * view);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        exportSceneFBO.unbind();

        PostProcessSettings exportPPSettings = ppSettings;
        exportPPSettings.lightningFlash = vfxSystem.getLightningFlash();
        exportPP.process(exportSceneFBO.colorTexture, exportPPSettings);

        // Read pixels
        glBindFramebuffer(GL_READ_FRAMEBUFFER, exportPP.getOutputTexture());
        // Actually need to bind the FBO that contains the output
        // The output texture is in the output FBO of postprocessor
        // Let's read from the output FBO texture
        exportFBO.bind();
        glClear(GL_COLOR_BUFFER_BIT);

        // Render the post-processed result to exportFBO
        Shader finalShader;
        finalShader.init(Shaders::COMPOSITE_VERT, Shaders::SCREEN_QUAD_FRAG);
        finalShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, exportPP.getOutputTexture());
        finalShader.setInt("uTexture", 0);
        finalShader.setFloat("uAlpha", 1.0f);
        quadRenderer.renderQuad();

        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        finalShader.destroy();
        exportFBO.unbind();

        // Flip vertically for FFmpeg
        std::vector<unsigned char> flipped(w * h * 3);
        for (int y = 0; y < h; y++) {
            std::copy(pixels.data() + (h - 1 - y) * w * 3,
                     pixels.data() + (h - y) * w * 3,
                     flipped.data() + y * w * 3);
        }

        exporter.writeFrame(flipped.data(), w, h);

        if (frame % 30 == 0) {
            float progress = exporter.getProgress(duration, fps) * 100.0f;
            editor.state.statusMessage = "Exporting: " + std::to_string(static_cast<int>(progress)) + "%";
        }
    }

    exporter.finish();
    exportFBO.destroy();
    exportPP.destroy();
    exportSceneFBO.destroy();

    editor.state.exporting = false;
    editor.state.statusMessage = "Export complete: " + editor.state.exportPath;
}

void Application::processInput() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->windowWidth = width;
        app->windowHeight = height;
    }
}

void Application::shutdown() {
    exporter.finish();
    editor.destroy();
    vfxSystem.destroy();
    postProcessor.destroy();
    sceneFBO.destroy();
    quadShader.destroy();
    quadRenderer.destroy();
    scene.destroy();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();

    std::cout << "Cinematic VFX Editor shutdown complete" << std::endl;
}

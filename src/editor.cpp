#include "editor.h"
#include "imgui.h"
#include <cstring>
#include <algorithm>
#include <cmath>

void Editor::init() {
    setupDarkTheme();
    std::strncpy(imagePathBuffer, "", sizeof(imagePathBuffer));
    std::strncpy(exportPathBuffer, "output.mp4", sizeof(exportPathBuffer));
}

void Editor::setupDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Professional dark cinematic theme
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.95f);
    colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.14f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.07f, 0.50f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.60f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.50f, 0.85f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.60f, 0.95f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.18f, 0.24f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.28f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.28f, 0.35f, 0.50f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.18f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.30f, 0.42f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
    colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.10f, 0.14f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.28f, 0.38f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.22f, 0.32f, 1.00f);
    colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);

    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.IndentSpacing = 16.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 8.0f;
}

void Editor::render(Scene& scene, VFXSystem& vfx, Camera& camera,
                    PostProcessSettings& ppSettings, float viewportWidth, float viewportHeight,
                    GLuint viewportTexture) {
    // Setup dockspace
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspaceId = ImGui::GetID("CinematicDockSpace");
    ImGui::DockSpace(dockspaceId, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);

    renderMenuBar(scene);

    ImGui::End();

    // Panels
    float sceneW = scene.loaded ? static_cast<float>(scene.imageWidth) : 800.0f;
    float sceneH = scene.loaded ? static_cast<float>(scene.imageHeight) : 600.0f;

    renderViewport(viewportTexture, viewportWidth, viewportHeight, vfx, camera);

    if (state.showEffectPalette) {
        renderEffectPalette(vfx, sceneW, sceneH);
    }

    if (state.showProperties) {
        renderProperties(vfx, ppSettings, camera);
    }

    if (state.showTimeline) {
        renderTimeline();
    }

    renderStatusBar();
}

void Editor::renderMenuBar(Scene& scene) {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Image...", "Ctrl+O")) {
                // Will trigger file dialog
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Export Video...", "Ctrl+E")) {
                state.showProperties = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Exit handled in application
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Effect Palette", nullptr, &state.showEffectPalette);
            ImGui::MenuItem("Properties", nullptr, &state.showProperties);
            ImGui::MenuItem("Timeline", nullptr, &state.showTimeline);
            ImGui::Separator();
            ImGui::MenuItem("Show Depth Map", nullptr, &state.showDepthMap);
            ImGui::MenuItem("Show Layers", nullptr, &state.showLayers);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("Reset Camera")) {
                // Reset handled in application
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void Editor::renderViewport(GLuint viewportTexture, float vpWidth, float vpHeight,
                            VFXSystem& vfx, Camera& camera) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x > 0 && avail.y > 0) {
        ImGui::Image((ImTextureID)(intptr_t)viewportTexture, avail, ImVec2(0, 1), ImVec2(1, 0));

        // Handle viewport interactions
        if (ImGui::IsItemHovered()) {
            ImGuiIO& io = ImGui::GetIO();

            // Scroll to zoom
            if (io.MouseWheel != 0.0f) {
                camera.zoomBy(io.MouseWheel);
            }

            // Middle mouse drag to pan
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
                camera.pan(glm::vec2(delta.x, -delta.y) * 0.5f);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
            }

            // Right click to add effect at position (if dragging from palette)
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                ImVec2 mousePos = ImGui::GetMousePos();
                ImVec2 itemPos = ImGui::GetItemRectMin();
                ImVec2 itemSize = ImGui::GetItemRectSize();

                float relX = (mousePos.x - itemPos.x) / itemSize.x;
                float relY = 1.0f - (mousePos.y - itemPos.y) / itemSize.y;

                float sceneX = relX * vpWidth;
                float sceneY = relY * vpHeight;

                // Store position for potential effect placement
                state.statusMessage = "Right-click at (" + std::to_string((int)sceneX) + ", " + std::to_string((int)sceneY) + ")";
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void Editor::renderEffectPalette(VFXSystem& vfx, float sceneW, float sceneH) {
    ImGui::Begin("Effects Palette", &state.showEffectPalette);

    ImGui::TextColored(ImVec4(0.6f, 0.75f, 1.0f, 1.0f), "Drag & Drop VFX");
    ImGui::Separator();
    ImGui::Spacing();

    // Image loading
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.5f, 1.0f), "Scene Image");
    ImGui::InputText("Path", imagePathBuffer, sizeof(imagePathBuffer));
    if (ImGui::Button("Load Image", ImVec2(-1, 30))) {
        if (std::strlen(imagePathBuffer) > 0) {
            loadImageRequested = true;
            requestedImagePath = imagePathBuffer;
        }
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.5f, 1.0f), "Add Effects");
    ImGui::Spacing();

    struct EffectButton {
        EffectType type;
        ImVec4 color;
        const char* icon;
    };

    EffectButton buttons[] = {
        {EffectType::Fire,      ImVec4(1.0f, 0.4f, 0.1f, 1.0f), "Fire"},
        {EffectType::Rain,      ImVec4(0.5f, 0.7f, 1.0f, 1.0f), "Rain"},
        {EffectType::Fog,       ImVec4(0.7f, 0.7f, 0.8f, 1.0f), "Fog"},
        {EffectType::Smoke,     ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "Smoke"},
        {EffectType::Embers,    ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Embers"},
        {EffectType::Snow,      ImVec4(0.9f, 0.9f, 1.0f, 1.0f), "Snow"},
        {EffectType::Lightning, ImVec4(0.9f, 0.9f, 1.0f, 1.0f), "Lightning"},
        {EffectType::Dust,      ImVec4(0.8f, 0.75f, 0.6f, 1.0f), "Dust"},
        {EffectType::GodRays,   ImVec4(1.0f, 0.95f, 0.8f, 1.0f), "God Rays"},
    };

    float buttonWidth = ImGui::GetContentRegionAvail().x;
    for (auto& btn : buttons) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(btn.color.x * 0.3f, btn.color.y * 0.3f, btn.color.z * 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btn.color.x * 0.5f, btn.color.y * 0.5f, btn.color.z * 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(btn.color.x * 0.7f, btn.color.y * 0.7f, btn.color.z * 0.7f, 1.0f));

        if (ImGui::Button(btn.icon, ImVec2(buttonWidth, 32))) {
            glm::vec2 pos(sceneW * 0.5f, sceneH * 0.5f);
            int id = vfx.addEffect(btn.type, pos);
            state.selectedEffectId = id;
            state.statusMessage = std::string("Added ") + btn.icon + " effect";
        }

        ImGui::PopStyleColor(3);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Effect list
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.5f, 1.0f), "Active Effects");
    ImGui::Spacing();

    for (auto& effect : vfx.effects) {
        bool isSelected = (state.selectedEffectId == effect.id);

        ImGui::PushID(effect.id);

        if (ImGui::Selectable(effect.name.c_str(), isSelected)) {
            state.selectedEffectId = effect.id;
        }

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 30);
        ImGui::Checkbox("##active", &effect.active);

        ImGui::PopID();
    }

    if (state.selectedEffectId >= 0 && ImGui::Button("Remove Selected", ImVec2(-1, 28))) {
        vfx.removeEffect(state.selectedEffectId);
        state.selectedEffectId = -1;
        state.statusMessage = "Effect removed";
    }

    ImGui::End();
}

void Editor::renderProperties(VFXSystem& vfx, PostProcessSettings& ppSettings, Camera& camera) {
    ImGui::Begin("Properties", &state.showProperties);

    if (ImGui::BeginTabBar("PropertiesTabs")) {
        // Effect properties
        if (ImGui::BeginTabItem("Effect")) {
            EffectInstance* selected = vfx.findEffect(state.selectedEffectId);

            if (selected) {
                ImGui::TextColored(ImVec4(0.6f, 0.75f, 1.0f, 1.0f), "%s", selected->name.c_str());
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::DragFloat2("Position", &selected->position.x, 1.0f);
                ImGui::DragFloat2("Area", &selected->area.x, 1.0f, 1.0f, 2000.0f);
                ImGui::SliderFloat("Intensity", &selected->intensity, 0.0f, 3.0f);
                ImGui::SliderFloat("Scale", &selected->scale, 0.1f, 10.0f);
                ImGui::ColorEdit3("Color", &selected->color.x);
                ImGui::SliderFloat("Depth", &selected->depth, 0.0f, 1.0f);
                ImGui::SliderFloat("Spawn Rate", &selected->spawnRate, 0.0f, 1000.0f);

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.5f, 1.0f), "Physics");
                ImGui::SliderFloat("Wind Dir", &selected->windDirection, -180.0f, 180.0f);
                ImGui::SliderFloat("Wind Str", &selected->windStrength, 0.0f, 5.0f);
                ImGui::SliderFloat("Turbulence", &selected->turbulence, 0.0f, 2.0f);

                if (selected->type == EffectType::Lightning) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.5f, 1.0f), "Lightning");
                    ImGui::SliderFloat("Flash Duration", &selected->flashDuration, 0.01f, 1.0f);
                    ImGui::SliderFloat("Flash Cooldown", &selected->flashCooldown, 0.5f, 15.0f);
                }
            } else {
                ImGui::TextDisabled("No effect selected");
                ImGui::TextWrapped("Click an effect in the palette list or add a new one.");
            }

            ImGui::EndTabItem();
        }

        // Post-processing
        if (ImGui::BeginTabItem("Post FX")) {
            ImGui::TextColored(ImVec4(0.6f, 0.75f, 1.0f, 1.0f), "Bloom");
            ImGui::SliderFloat("Threshold", &ppSettings.bloomThreshold, 0.0f, 2.0f);
            ImGui::SliderFloat("Bloom Intensity", &ppSettings.bloomIntensity, 0.0f, 3.0f);
            ImGui::SliderInt("Bloom Passes", &ppSettings.bloomPasses, 1, 10);

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.6f, 0.75f, 1.0f, 1.0f), "Color Grading");
            ImGui::SliderFloat("Exposure", &ppSettings.exposure, 0.1f, 5.0f);
            ImGui::SliderFloat("Contrast", &ppSettings.contrast, 0.5f, 2.0f);
            ImGui::SliderFloat("Saturation", &ppSettings.saturation, 0.0f, 2.0f);
            ImGui::ColorEdit3("Tint", &ppSettings.tint.x);

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.6f, 0.75f, 1.0f, 1.0f), "Vignette");
            ImGui::SliderFloat("Vignette", &ppSettings.vignetteStrength, 0.0f, 1.0f);

            ImGui::EndTabItem();
        }

        // Camera
        if (ImGui::BeginTabItem("Camera")) {
            renderCameraControls(camera);
            ImGui::EndTabItem();
        }

        // Export
        if (ImGui::BeginTabItem("Export")) {
            renderExportPanel();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Editor::renderCameraControls(Camera& camera) {
    ImGui::TextColored(ImVec4(0.6f, 0.75f, 1.0f, 1.0f), "Camera Controls");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::DragFloat2("Position##cam", &camera.position.x, 0.5f);
    ImGui::SliderFloat("Zoom", &camera.targetZoom, 0.2f, 5.0f);
    ImGui::SliderFloat("Rotation##cam", &camera.rotation, -3.14159f, 3.14159f);

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.5f, 1.0f), "Cinematic Drift");
    ImGui::Checkbox("Enable Drift", &camera.settings.enableDrift);
    ImGui::SliderFloat("Drift Speed", &camera.settings.driftSpeed, 0.0f, 1.0f);
    ImGui::SliderFloat("Drift Amount", &camera.settings.driftAmount, 0.0f, 100.0f);

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.5f, 1.0f), "Camera Shake");
    ImGui::Checkbox("Enable Shake", &camera.settings.enableShake);
    ImGui::SliderFloat("Shake Intensity", &camera.settings.shakeIntensity, 0.0f, 10.0f);
    ImGui::SliderFloat("Shake Frequency", &camera.settings.shakeFrequency, 1.0f, 20.0f);

    ImGui::Spacing();
    if (ImGui::Button("Reset Camera", ImVec2(-1, 30))) {
        camera.reset();
    }
}

void Editor::renderTimeline() {
    ImGui::Begin("Timeline", &state.showTimeline);

    // Playback controls
    if (ImGui::Button(state.isPlaying ? "Pause" : "Play", ImVec2(60, 28))) {
        state.isPlaying = !state.isPlaying;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(60, 28))) {
        state.timelinePosition = 0.0f;
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("##timeline", &state.timelinePosition, 0.0f, state.timelineDuration, "%.1f s");

    // Duration control
    ImGui::SetNextItemWidth(120);
    ImGui::DragFloat("Duration", &state.timelineDuration, 0.5f, 1.0f, 300.0f, "%.1f s");

    ImGui::End();
}

void Editor::renderExportPanel() {
    ImGui::TextColored(ImVec4(0.6f, 0.75f, 1.0f, 1.0f), "Video Export");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::InputText("Output Path", exportPathBuffer, sizeof(exportPathBuffer));
    ImGui::InputInt("Width", &state.exportWidth, 16);
    ImGui::InputInt("Height", &state.exportHeight, 16);
    ImGui::InputInt("FPS", &state.exportFPS, 1);
    ImGui::DragFloat("Duration##export", &state.exportDuration, 0.5f, 1.0f, 300.0f, "%.1f s");

    state.exportWidth = std::max(state.exportWidth, 64);
    state.exportHeight = std::max(state.exportHeight, 64);
    state.exportFPS = std::clamp(state.exportFPS, 1, 120);

    ImGui::Spacing();
    if (!state.exporting) {
        if (ImGui::Button("Export Video", ImVec2(-1, 35))) {
            state.exportPath = exportPathBuffer;
            exportRequested = true;
            state.statusMessage = "Starting export...";
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Exporting...");
    }
}

void Editor::renderStatusBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 statusPos(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - 24);
    ImVec2 statusSize(viewport->WorkSize.x, 24);

    ImGui::SetNextWindowPos(statusPos);
    ImGui::SetNextWindowSize(statusSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 3));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.08f, 1.0f));

    ImGui::Begin("##StatusBar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoDocking);

    ImGui::TextColored(ImVec4(0.5f, 0.6f, 0.8f, 1.0f), "Cinematic VFX Editor");
    ImGui::SameLine(200);
    if (!state.statusMessage.empty()) {
        ImGui::Text("%s", state.statusMessage.c_str());
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void Editor::destroy() {
    // Nothing to destroy
}

#pragma once
#include "scene.h"
#include "vfx_system.h"
#include "camera.h"
#include "post_process.h"
#include <string>

struct EditorState {
    int selectedEffectId = -1;
    bool showDepthMap = false;
    bool showLayers = false;
    bool showTimeline = true;
    bool showProperties = true;
    bool showEffectPalette = true;
    bool isPlaying = true;
    float timelinePosition = 0.0f;
    float timelineDuration = 10.0f;
    int exportWidth = 1920;
    int exportHeight = 1080;
    int exportFPS = 30;
    float exportDuration = 10.0f;
    bool exporting = false;
    std::string exportPath = "output.mp4";
    std::string statusMessage;
};

class Editor {
public:
    EditorState state;

    void init();
    void render(Scene& scene, VFXSystem& vfx, Camera& camera,
                PostProcessSettings& ppSettings, float viewportWidth, float viewportHeight,
                GLuint viewportTexture);
    void destroy();

    bool wantsLoadImage() const { return loadImageRequested; }
    std::string getRequestedImagePath() const { return requestedImagePath; }
    void clearLoadRequest() { loadImageRequested = false; requestedImagePath.clear(); }
    bool wantsExport() const { return exportRequested; }
    void clearExportRequest() { exportRequested = false; }

private:
    bool loadImageRequested = false;
    std::string requestedImagePath;
    bool exportRequested = false;
    char imagePathBuffer[512] = {};
    char exportPathBuffer[512] = "output.mp4";

    void setupDarkTheme();
    void renderMenuBar(Scene& scene);
    void renderViewport(GLuint viewportTexture, float vpWidth, float vpHeight,
                       VFXSystem& vfx, Camera& camera);
    void renderEffectPalette(VFXSystem& vfx, float sceneW, float sceneH);
    void renderProperties(VFXSystem& vfx, PostProcessSettings& ppSettings, Camera& camera);
    void renderTimeline();
    void renderLayerPanel(Scene& scene);
    void renderExportPanel();
    void renderStatusBar();
    void renderCameraControls(Camera& camera);
};

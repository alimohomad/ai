#pragma once
#include "gl_utils.h"
#include <glm/glm.hpp>

struct PostProcessSettings {
    float bloomThreshold = 0.6f;
    float bloomIntensity = 0.4f;
    int bloomPasses = 5;
    float exposure = 1.2f;
    float contrast = 1.1f;
    float saturation = 1.0f;
    glm::vec3 tint{1.0f, 1.0f, 1.0f};
    float vignetteStrength = 0.3f;
    float lightningFlash = 0.0f;
};

class PostProcessor {
public:
    void init(int width, int height);
    void resize(int width, int height);
    void process(GLuint sceneTexture, const PostProcessSettings& settings);
    GLuint getOutputTexture() const;
    void destroy();

private:
    int width = 0, height = 0;
    Framebuffer brightFBO;
    Framebuffer pingFBO, pongFBO;
    Framebuffer outputFBO;

    Shader brightShader;
    Shader blurShader;
    Shader compositeShader;
    QuadRenderer quad;

    void extractBright(GLuint sceneTexture, float threshold);
    void blurPass(int passes);
    void composite(GLuint sceneTexture, const PostProcessSettings& settings);
};

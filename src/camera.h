#pragma once
#include <glm/glm.hpp>

struct CameraSettings {
    bool enableDrift = true;
    float driftSpeed = 0.15f;
    float driftAmount = 20.0f;
    float zoomSpeed = 0.1f;
    bool enableShake = false;
    float shakeIntensity = 2.0f;
    float shakeFrequency = 8.0f;
    float depthOfField = 0.0f;
    float focalDistance = 0.5f;
};

class Camera {
public:
    glm::vec2 position{0.0f};
    float zoom = 1.0f;
    float targetZoom = 1.0f;
    float rotation = 0.0f;

    CameraSettings settings;

    void update(float dt);
    void reset();
    glm::mat4 getViewMatrix(float sceneWidth, float sceneHeight) const;
    glm::mat4 getProjectionMatrix(float viewportWidth, float viewportHeight) const;
    glm::vec2 getParallaxOffset(float depthLayer) const;

    void pan(const glm::vec2& delta);
    void zoomBy(float amount);

private:
    float time = 0.0f;
    glm::vec2 shakeOffset{0.0f};
    glm::vec2 driftOffset{0.0f};
};

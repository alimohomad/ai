#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

void Camera::update(float dt) {
    time += dt;

    if (settings.enableDrift) {
        driftOffset.x = std::sin(time * settings.driftSpeed * 0.7f) * settings.driftAmount;
        driftOffset.y = std::cos(time * settings.driftSpeed) * settings.driftAmount * 0.5f;
    } else {
        driftOffset = glm::vec2(0.0f);
    }

    if (settings.enableShake) {
        float freq = settings.shakeFrequency;
        shakeOffset.x = std::sin(time * freq * 1.1f) * settings.shakeIntensity;
        shakeOffset.y = std::cos(time * freq * 0.9f) * settings.shakeIntensity;
    } else {
        shakeOffset = glm::vec2(0.0f);
    }

    zoom += (targetZoom - zoom) * 0.05f;
}

void Camera::reset() {
    position = glm::vec2(0.0f);
    zoom = 1.0f;
    targetZoom = 1.0f;
    rotation = 0.0f;
    time = 0.0f;
}

glm::mat4 Camera::getViewMatrix(float sceneWidth, float sceneHeight) const {
    glm::vec2 totalOffset = position + driftOffset + shakeOffset;

    glm::mat4 view(1.0f);
    view = glm::translate(view, glm::vec3(sceneWidth * 0.5f, sceneHeight * 0.5f, 0.0f));
    view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));
    view = glm::rotate(view, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::translate(view, glm::vec3(-sceneWidth * 0.5f - totalOffset.x, -sceneHeight * 0.5f - totalOffset.y, 0.0f));

    return view;
}

glm::mat4 Camera::getProjectionMatrix(float viewportWidth, float viewportHeight) const {
    return glm::ortho(0.0f, viewportWidth, 0.0f, viewportHeight, -1.0f, 1.0f);
}

glm::vec2 Camera::getParallaxOffset(float depthLayer) const {
    float parallaxFactor = (1.0f - depthLayer) * 0.3f;
    glm::vec2 totalOffset = position + driftOffset + shakeOffset;
    return totalOffset * parallaxFactor;
}

void Camera::pan(const glm::vec2& delta) {
    position += delta / zoom;
}

void Camera::zoomBy(float amount) {
    targetZoom += amount * settings.zoomSpeed;
    targetZoom = glm::clamp(targetZoom, 0.2f, 5.0f);
}

#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

enum class EffectType {
    Fire,
    Rain,
    Fog,
    Smoke,
    Embers,
    Snow,
    Lightning,
    Dust,
    GodRays,
    COUNT
};

const char* effectTypeName(EffectType type);

struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
    float size;
    float life;
    float maxLife;
    float rotation;
    float rotationSpeed;
};

struct EffectInstance {
    EffectType type = EffectType::Fire;
    glm::vec2 position{400.0f, 300.0f};
    glm::vec2 area{100.0f, 50.0f};
    float intensity = 1.0f;
    float scale = 1.0f;
    glm::vec3 color{1.0f, 0.5f, 0.1f};
    bool active = true;
    bool selected = false;
    std::string name;
    float depth = 0.5f;

    float windDirection = 0.0f;
    float windStrength = 0.0f;
    float turbulence = 0.3f;
    float spawnRate = 100.0f;

    // Lightning-specific
    float flashTimer = 0.0f;
    float flashDuration = 0.15f;
    float flashCooldown = 3.0f;
    float flashIntensity = 0.0f;

    int id = 0;
};

class ParticleRenderer {
public:
    void init();
    void destroy();
    void begin(const glm::mat4& projection);
    void renderParticles(const std::vector<Particle>& particles, int shape);
    void end();

private:
    GLuint vao = 0, quadVBO = 0, instanceVBO = 0;
    GLuint shaderProgram = 0;
    size_t maxInstances = 50000;

    struct ParticleInstanceData {
        glm::vec2 offset;
        glm::vec4 color;
        float size;
        float rotation;
    };
};

class VFXSystem {
public:
    std::vector<EffectInstance> effects;
    std::vector<Particle> particles;

    void init();
    void update(float dt, float sceneWidth, float sceneHeight);
    void render(const glm::mat4& projection);
    void destroy();

    int addEffect(EffectType type, const glm::vec2& pos);
    void removeEffect(int id);
    EffectInstance* findEffect(int id);
    float getLightningFlash() const;

private:
    ParticleRenderer renderer;
    int nextId = 1;
    float globalTime = 0.0f;

    void spawnParticles(EffectInstance& effect, float dt, float sceneWidth, float sceneHeight);
    void updateParticles(float dt);

    void spawnFire(EffectInstance& e, float dt);
    void spawnRain(EffectInstance& e, float dt, float sw, float sh);
    void spawnFog(EffectInstance& e, float dt, float sw, float sh);
    void spawnSmoke(EffectInstance& e, float dt);
    void spawnEmbers(EffectInstance& e, float dt);
    void spawnSnow(EffectInstance& e, float dt, float sw, float sh);
    void spawnDust(EffectInstance& e, float dt, float sw, float sh);
    void updateLightning(EffectInstance& e, float dt);
};

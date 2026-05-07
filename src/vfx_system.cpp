#include "vfx_system.h"
#include "shaders.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>

static std::mt19937 rng(42);
static std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
static std::uniform_real_distribution<float> distNeg(-1.0f, 1.0f);

static float randFloat(float min, float max) {
    return min + dist01(rng) * (max - min);
}

const char* effectTypeName(EffectType type) {
    switch (type) {
        case EffectType::Fire: return "Fire";
        case EffectType::Rain: return "Rain";
        case EffectType::Fog: return "Fog";
        case EffectType::Smoke: return "Smoke";
        case EffectType::Embers: return "Embers";
        case EffectType::Snow: return "Snow";
        case EffectType::Lightning: return "Lightning";
        case EffectType::Dust: return "Dust";
        case EffectType::GodRays: return "God Rays";
        default: return "Unknown";
    }
}

// --- ParticleRenderer ---

void ParticleRenderer::init() {
    float quadVerts[] = {
        -0.5f,  0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, maxInstances * sizeof(ParticleInstanceData), nullptr, GL_DYNAMIC_DRAW);

    // offset
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)0);
    glVertexAttribDivisor(2, 1);

    // color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)(2 * sizeof(float)));
    glVertexAttribDivisor(3, 1);

    // size
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)(6 * sizeof(float)));
    glVertexAttribDivisor(4, 1);

    // rotation
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)(7 * sizeof(float)));
    glVertexAttribDivisor(5, 1);

    glBindVertexArray(0);

    // Compile particle shader
    const char* vertSrc = Shaders::PARTICLE_VERT.c_str();
    const char* fragSrc = Shaders::PARTICLE_FRAG.c_str();

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertSrc, nullptr);
    glCompileShader(vert);

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragSrc, nullptr);
    glCompileShader(frag);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vert);
    glAttachShader(shaderProgram, frag);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, log);
        std::cerr << "Particle shader link error: " << log << std::endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

void ParticleRenderer::destroy() {
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (instanceVBO) glDeleteBuffers(1, &instanceVBO);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    quadVBO = instanceVBO = vao = shaderProgram = 0;
}

void ParticleRenderer::begin(const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    GLint loc = glGetUniformLocation(shaderProgram, "uProjection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &projection[0][0]);
}

void ParticleRenderer::renderParticles(const std::vector<Particle>& particles, int shape) {
    if (particles.empty()) return;

    size_t count = std::min(particles.size(), maxInstances);
    std::vector<ParticleInstanceData> instances(count);

    for (size_t i = 0; i < count; i++) {
        instances[i].offset = particles[i].position;
        instances[i].color = particles[i].color;
        instances[i].size = particles[i].size;
        instances[i].rotation = particles[i].rotation;
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(ParticleInstanceData), instances.data());

    GLint shapeLoc = glGetUniformLocation(shaderProgram, "uShape");
    glUniform1i(shapeLoc, shape);

    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(count));
    glBindVertexArray(0);
}

void ParticleRenderer::end() {
    glUseProgram(0);
}

// --- VFXSystem ---

void VFXSystem::init() {
    renderer.init();
}

void VFXSystem::destroy() {
    renderer.destroy();
    effects.clear();
    particles.clear();
}

int VFXSystem::addEffect(EffectType type, const glm::vec2& pos) {
    EffectInstance effect;
    effect.type = type;
    effect.position = pos;
    effect.id = nextId++;
    effect.name = std::string(effectTypeName(type)) + " " + std::to_string(effect.id);

    switch (type) {
        case EffectType::Fire:
            effect.color = glm::vec3(1.0f, 0.4f, 0.05f);
            effect.area = glm::vec2(80.0f, 40.0f);
            effect.spawnRate = 200.0f;
            effect.turbulence = 0.4f;
            break;
        case EffectType::Rain:
            effect.color = glm::vec3(0.7f, 0.8f, 0.95f);
            effect.area = glm::vec2(800.0f, 600.0f);
            effect.spawnRate = 500.0f;
            effect.intensity = 0.6f;
            break;
        case EffectType::Fog:
            effect.color = glm::vec3(0.8f, 0.85f, 0.9f);
            effect.area = glm::vec2(600.0f, 200.0f);
            effect.spawnRate = 30.0f;
            effect.intensity = 0.3f;
            effect.scale = 3.0f;
            break;
        case EffectType::Smoke:
            effect.color = glm::vec3(0.4f, 0.4f, 0.45f);
            effect.area = glm::vec2(60.0f, 30.0f);
            effect.spawnRate = 80.0f;
            effect.turbulence = 0.5f;
            break;
        case EffectType::Embers:
            effect.color = glm::vec3(1.0f, 0.6f, 0.1f);
            effect.area = glm::vec2(100.0f, 60.0f);
            effect.spawnRate = 40.0f;
            break;
        case EffectType::Snow:
            effect.color = glm::vec3(0.95f, 0.95f, 1.0f);
            effect.area = glm::vec2(800.0f, 600.0f);
            effect.spawnRate = 150.0f;
            effect.intensity = 0.8f;
            break;
        case EffectType::Lightning:
            effect.color = glm::vec3(0.9f, 0.9f, 1.0f);
            effect.flashCooldown = 4.0f;
            effect.flashDuration = 0.12f;
            effect.spawnRate = 0.0f;
            break;
        case EffectType::Dust:
            effect.color = glm::vec3(0.9f, 0.85f, 0.7f);
            effect.area = glm::vec2(400.0f, 300.0f);
            effect.spawnRate = 60.0f;
            effect.intensity = 0.4f;
            break;
        case EffectType::GodRays:
            effect.color = glm::vec3(1.0f, 0.95f, 0.8f);
            effect.area = glm::vec2(200.0f, 400.0f);
            effect.spawnRate = 20.0f;
            effect.intensity = 0.3f;
            effect.scale = 5.0f;
            break;
        default: break;
    }

    effects.push_back(effect);
    return effect.id;
}

void VFXSystem::removeEffect(int id) {
    effects.erase(std::remove_if(effects.begin(), effects.end(),
        [id](const EffectInstance& e) { return e.id == id; }), effects.end());
}

EffectInstance* VFXSystem::findEffect(int id) {
    for (auto& e : effects) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

float VFXSystem::getLightningFlash() const {
    float maxFlash = 0.0f;
    for (const auto& e : effects) {
        if (e.type == EffectType::Lightning && e.active) {
            maxFlash = std::max(maxFlash, e.flashIntensity);
        }
    }
    return maxFlash;
}

void VFXSystem::update(float dt, float sceneWidth, float sceneHeight) {
    globalTime += dt;

    for (auto& effect : effects) {
        if (!effect.active) continue;
        if (effect.type == EffectType::Lightning) {
            updateLightning(effect, dt);
        } else {
            spawnParticles(effect, dt, sceneWidth, sceneHeight);
        }
    }

    updateParticles(dt);
}

void VFXSystem::updateParticles(float dt) {
    for (auto& p : particles) {
        p.position += p.velocity * dt;
        p.life -= dt;
        p.rotation += p.rotationSpeed * dt;

        float lifeRatio = p.life / p.maxLife;
        p.color.a = lifeRatio;
    }

    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& p) { return p.life <= 0.0f; }), particles.end());
}

void VFXSystem::spawnParticles(EffectInstance& effect, float dt, float sw, float sh) {
    float rate = effect.spawnRate * effect.intensity;
    int count = static_cast<int>(rate * dt);
    if (dist01(rng) < (rate * dt - count)) count++;

    for (int i = 0; i < count && particles.size() < 50000; i++) {
        switch (effect.type) {
            case EffectType::Fire: spawnFire(effect, dt); break;
            case EffectType::Rain: spawnRain(effect, dt, sw, sh); break;
            case EffectType::Fog: spawnFog(effect, dt, sw, sh); break;
            case EffectType::Smoke: spawnSmoke(effect, dt); break;
            case EffectType::Embers: spawnEmbers(effect, dt); break;
            case EffectType::Snow: spawnSnow(effect, dt, sw, sh); break;
            case EffectType::Dust: spawnDust(effect, dt, sw, sh); break;
            case EffectType::GodRays: {
                Particle p;
                p.position = effect.position + glm::vec2(
                    randFloat(-effect.area.x * 0.5f, effect.area.x * 0.5f),
                    randFloat(0.0f, effect.area.y)
                );
                p.velocity = glm::vec2(0.0f, -20.0f);
                float alpha = effect.intensity * 0.15f;
                p.color = glm::vec4(effect.color, alpha);
                p.size = randFloat(30.0f, 80.0f) * effect.scale;
                p.life = p.maxLife = randFloat(2.0f, 4.0f);
                p.rotation = 0.0f;
                p.rotationSpeed = 0.0f;
                particles.push_back(p);
                break;
            }
            default: break;
        }
    }
}

void VFXSystem::spawnFire(EffectInstance& e, float dt) {
    Particle p;
    p.position = e.position + glm::vec2(
        randFloat(-e.area.x * 0.5f, e.area.x * 0.5f),
        randFloat(-e.area.y * 0.3f, e.area.y * 0.1f)
    );

    float angle = randFloat(-0.3f, 0.3f);
    float speed = randFloat(40.0f, 120.0f);
    p.velocity = glm::vec2(std::sin(angle) * speed * 0.3f, speed);
    p.velocity.x += std::sin(globalTime * 3.0f + p.position.x * 0.01f) * 15.0f * e.turbulence;

    float t = dist01(rng);
    glm::vec3 innerColor(1.0f, 0.9f, 0.3f);
    glm::vec3 outerColor = e.color;
    glm::vec3 col = glm::mix(innerColor, outerColor, t);
    p.color = glm::vec4(col, e.intensity * randFloat(0.5f, 1.0f));

    p.size = randFloat(3.0f, 12.0f) * e.scale;
    p.life = p.maxLife = randFloat(0.3f, 1.0f);
    p.rotation = randFloat(0.0f, 6.28f);
    p.rotationSpeed = randFloat(-2.0f, 2.0f);

    particles.push_back(p);
}

void VFXSystem::spawnRain(EffectInstance& e, float dt, float sw, float sh) {
    Particle p;
    p.position = glm::vec2(
        e.position.x + randFloat(-e.area.x * 0.5f, e.area.x * 0.5f),
        e.position.y + e.area.y * 0.5f + randFloat(0.0f, 50.0f)
    );

    float windAngle = e.windDirection * 3.14159f / 180.0f;
    p.velocity = glm::vec2(
        std::sin(windAngle) * 50.0f * (1.0f + e.windStrength),
        -(400.0f + randFloat(0.0f, 200.0f))
    );

    p.color = glm::vec4(e.color, e.intensity * randFloat(0.2f, 0.5f));
    p.size = randFloat(1.0f, 3.0f) * e.scale;
    p.life = p.maxLife = randFloat(0.5f, 1.5f);
    p.rotation = std::atan2(p.velocity.x, -p.velocity.y);
    p.rotationSpeed = 0.0f;

    particles.push_back(p);
}

void VFXSystem::spawnFog(EffectInstance& e, float dt, float sw, float sh) {
    Particle p;
    p.position = e.position + glm::vec2(
        randFloat(-e.area.x * 0.5f, e.area.x * 0.5f),
        randFloat(-e.area.y * 0.3f, e.area.y * 0.3f)
    );

    p.velocity = glm::vec2(
        randFloat(5.0f, 20.0f) + e.windStrength * 10.0f,
        randFloat(-3.0f, 3.0f)
    );

    p.color = glm::vec4(e.color, e.intensity * randFloat(0.05f, 0.15f));
    p.size = randFloat(80.0f, 200.0f) * e.scale;
    p.life = p.maxLife = randFloat(5.0f, 12.0f);
    p.rotation = randFloat(0.0f, 6.28f);
    p.rotationSpeed = randFloat(-0.1f, 0.1f);

    particles.push_back(p);
}

void VFXSystem::spawnSmoke(EffectInstance& e, float dt) {
    Particle p;
    p.position = e.position + glm::vec2(
        randFloat(-e.area.x * 0.3f, e.area.x * 0.3f),
        randFloat(-5.0f, 5.0f)
    );

    p.velocity = glm::vec2(
        randFloat(-10.0f, 10.0f) + e.windStrength * 5.0f,
        randFloat(30.0f, 80.0f)
    );
    p.velocity.x += std::sin(globalTime * 2.0f) * 8.0f * e.turbulence;

    p.color = glm::vec4(e.color, e.intensity * randFloat(0.15f, 0.35f));
    p.size = randFloat(15.0f, 40.0f) * e.scale;
    p.life = p.maxLife = randFloat(2.0f, 5.0f);
    p.rotation = randFloat(0.0f, 6.28f);
    p.rotationSpeed = randFloat(-0.5f, 0.5f);

    particles.push_back(p);
}

void VFXSystem::spawnEmbers(EffectInstance& e, float dt) {
    Particle p;
    p.position = e.position + glm::vec2(
        randFloat(-e.area.x * 0.5f, e.area.x * 0.5f),
        randFloat(-e.area.y * 0.3f, e.area.y * 0.3f)
    );

    float angle = randFloat(0.5f, 2.6f);
    float speed = randFloat(20.0f, 60.0f);
    p.velocity = glm::vec2(std::cos(angle) * speed * 0.5f, std::sin(angle) * speed);
    p.velocity.x += std::sin(globalTime * 4.0f + p.position.y * 0.02f) * 10.0f;

    float brightness = randFloat(0.6f, 1.0f);
    p.color = glm::vec4(e.color * brightness, e.intensity * randFloat(0.6f, 1.0f));
    p.size = randFloat(1.5f, 4.0f) * e.scale;
    p.life = p.maxLife = randFloat(1.0f, 3.0f);
    p.rotation = 0.0f;
    p.rotationSpeed = randFloat(-3.0f, 3.0f);

    particles.push_back(p);
}

void VFXSystem::spawnSnow(EffectInstance& e, float dt, float sw, float sh) {
    Particle p;
    p.position = glm::vec2(
        e.position.x + randFloat(-e.area.x * 0.5f, e.area.x * 0.5f),
        e.position.y + e.area.y * 0.5f + randFloat(0.0f, 20.0f)
    );

    p.velocity = glm::vec2(
        randFloat(-15.0f, 15.0f) + e.windStrength * 10.0f,
        -(30.0f + randFloat(0.0f, 40.0f))
    );

    p.color = glm::vec4(e.color, e.intensity * randFloat(0.5f, 0.9f));
    p.size = randFloat(2.0f, 6.0f) * e.scale;
    p.life = p.maxLife = randFloat(3.0f, 8.0f);
    p.rotation = randFloat(0.0f, 6.28f);
    p.rotationSpeed = randFloat(-1.0f, 1.0f);

    particles.push_back(p);
}

void VFXSystem::spawnDust(EffectInstance& e, float dt, float sw, float sh) {
    Particle p;
    p.position = e.position + glm::vec2(
        randFloat(-e.area.x * 0.5f, e.area.x * 0.5f),
        randFloat(-e.area.y * 0.5f, e.area.y * 0.5f)
    );

    p.velocity = glm::vec2(
        randFloat(-5.0f, 5.0f),
        randFloat(-3.0f, 3.0f)
    );

    p.color = glm::vec4(e.color, e.intensity * randFloat(0.1f, 0.3f));
    p.size = randFloat(1.0f, 3.0f) * e.scale;
    p.life = p.maxLife = randFloat(3.0f, 8.0f);
    p.rotation = 0.0f;
    p.rotationSpeed = 0.0f;

    particles.push_back(p);
}

void VFXSystem::updateLightning(EffectInstance& e, float dt) {
    e.flashTimer += dt;
    if (e.flashTimer >= e.flashCooldown) {
        e.flashTimer = 0.0f;
        e.flashIntensity = 1.0f;
    }

    if (e.flashIntensity > 0.0f) {
        e.flashIntensity -= dt / e.flashDuration;
        if (e.flashIntensity < 0.0f) e.flashIntensity = 0.0f;
    }
}

void VFXSystem::render(const glm::mat4& projection) {
    if (particles.empty()) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    renderer.begin(projection);

    // Group particles by effect type for proper shape rendering
    // Fire, embers, smoke, fog, god rays: soft glow (shape 2)
    // Rain: streak (shape 1)
    // Snow, dust: circle (shape 0)

    std::vector<Particle> circles, streaks, glows, sparks;

    for (const auto& p : particles) {
        glows.push_back(p);
    }

    // Render different categories
    // For simplicity, use a single shape for now based on most common
    // In a full implementation, we'd tag particles with their effect type

    if (!glows.empty()) renderer.renderParticles(glows, 2);

    renderer.end();
}

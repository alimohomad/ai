#pragma once
#include <string>

namespace Shaders {

inline const std::string SCREEN_QUAD_VERT = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 uProjection;
uniform mat4 uModel;

void main() {
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

inline const std::string SCREEN_QUAD_FRAG = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uAlpha;

void main() {
    vec4 color = texture(uTexture, TexCoord);
    FragColor = vec4(color.rgb, color.a * uAlpha);
}
)";

inline const std::string PARTICLE_VERT = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec2 aOffset;
layout (location = 3) in vec4 aColor;
layout (location = 4) in float aSize;
layout (location = 5) in float aRotation;

out vec2 TexCoord;
out vec4 ParticleColor;

uniform mat4 uProjection;

void main() {
    float c = cos(aRotation);
    float s = sin(aRotation);
    mat2 rot = mat2(c, s, -s, c);
    vec2 pos = rot * (aPos * aSize) + aOffset;
    gl_Position = uProjection * vec4(pos, 0.0, 1.0);
    TexCoord = aTexCoord;
    ParticleColor = aColor;
}
)";

inline const std::string PARTICLE_FRAG = R"(
#version 330 core
in vec2 TexCoord;
in vec4 ParticleColor;
out vec4 FragColor;

uniform int uShape; // 0=circle, 1=streak, 2=soft_glow, 3=spark

void main() {
    vec2 uv = TexCoord * 2.0 - 1.0;
    float alpha = 0.0;

    if (uShape == 0) { // circle
        float d = length(uv);
        alpha = smoothstep(1.0, 0.3, d);
    } else if (uShape == 1) { // streak (rain)
        float dx = abs(uv.x);
        float dy = abs(uv.y);
        alpha = smoothstep(1.0, 0.0, dx * 4.0) * smoothstep(1.0, 0.0, dy);
    } else if (uShape == 2) { // soft glow
        float d = length(uv);
        alpha = exp(-d * d * 2.0);
    } else if (uShape == 3) { // spark
        float d = length(uv);
        alpha = smoothstep(1.0, 0.0, d) * smoothstep(0.0, 0.1, d);
        alpha = max(alpha, exp(-d * d * 8.0) * 0.8);
    }

    FragColor = vec4(ParticleColor.rgb, ParticleColor.a * alpha);
}
)";

inline const std::string BLOOM_BRIGHT_FRAG = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uThreshold;

void main() {
    vec4 color = texture(uTexture, TexCoord);
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > uThreshold) {
        FragColor = color;
    } else {
        FragColor = vec4(0.0);
    }
}
)";

inline const std::string BLOOM_BLUR_FRAG = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool uHorizontal;
uniform float uTexelSize;

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 offset = uHorizontal ? vec2(uTexelSize, 0.0) : vec2(0.0, uTexelSize);
    vec3 result = texture(uTexture, TexCoord).rgb * weights[0];

    for (int i = 1; i < 5; ++i) {
        result += texture(uTexture, TexCoord + offset * float(i)).rgb * weights[i];
        result += texture(uTexture, TexCoord - offset * float(i)).rgb * weights[i];
    }

    FragColor = vec4(result, 1.0);
}
)";

inline const std::string COMPOSITE_VERT = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

inline const std::string COMPOSITE_FRAG = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uScene;
uniform sampler2D uBloom;
uniform float uBloomIntensity;
uniform float uExposure;
uniform float uContrast;
uniform float uSaturation;
uniform vec3 uTint;
uniform float uVignetteStrength;

void main() {
    vec3 scene = texture(uScene, TexCoord).rgb;
    vec3 bloom = texture(uBloom, TexCoord).rgb;

    vec3 color = scene + bloom * uBloomIntensity;

    // Exposure tone mapping
    color = vec3(1.0) - exp(-color * uExposure);

    // Contrast
    color = ((color - 0.5) * uContrast) + 0.5;

    // Saturation
    float gray = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(gray), color, uSaturation);

    // Tint
    color *= uTint;

    // Vignette
    vec2 uv = TexCoord * 2.0 - 1.0;
    float vignette = 1.0 - dot(uv, uv) * uVignetteStrength;
    color *= clamp(vignette, 0.0, 1.0);

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
)";

inline const std::string DEPTH_VIS_FRAG = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    float depth = texture(uTexture, TexCoord).r;
    FragColor = vec4(vec3(depth), 1.0);
}
)";

inline const std::string LIGHTNING_FRAG = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform float uFlashIntensity;
uniform vec3 uFlashColor;

void main() {
    FragColor = vec4(uFlashColor * uFlashIntensity, uFlashIntensity * 0.3);
}
)";

} // namespace Shaders

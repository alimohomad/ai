#include "scene.h"
#include "stb_image.h"
#include <cmath>
#include <algorithm>
#include <iostream>

static float sobelMagnitude(const unsigned char* data, int x, int y, int w, int h, int ch) {
    auto pixel = [&](int px, int py) -> float {
        px = std::clamp(px, 0, w - 1);
        py = std::clamp(py, 0, h - 1);
        int idx = (py * w + px) * ch;
        float lum = 0.0f;
        if (ch >= 3) {
            lum = data[idx] * 0.2126f + data[idx + 1] * 0.7152f + data[idx + 2] * 0.0722f;
        } else {
            lum = data[idx];
        }
        return lum / 255.0f;
    };

    float gx = -pixel(x - 1, y - 1) + pixel(x + 1, y - 1)
             - 2.0f * pixel(x - 1, y) + 2.0f * pixel(x + 1, y)
             - pixel(x - 1, y + 1) + pixel(x + 1, y + 1);

    float gy = -pixel(x - 1, y - 1) - 2.0f * pixel(x, y - 1) - pixel(x + 1, y - 1)
             + pixel(x - 1, y + 1) + 2.0f * pixel(x, y + 1) + pixel(x + 1, y + 1);

    return std::sqrt(gx * gx + gy * gy);
}

void DepthMap::generate(const unsigned char* imageData, int w, int h, int channels) {
    width = w;
    height = h;
    data.resize(w * h);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float normalizedY = static_cast<float>(y) / static_cast<float>(h);
            float baseDepth = normalizedY;

            int idx = (y * w + x) * channels;
            float lum = 0.0f;
            if (channels >= 3) {
                lum = imageData[idx] * 0.2126f + imageData[idx + 1] * 0.7152f + imageData[idx + 2] * 0.0722f;
            } else {
                lum = imageData[idx];
            }
            lum /= 255.0f;

            float edge = sobelMagnitude(imageData, x, y, w, h, channels);

            float saturation = 0.0f;
            if (channels >= 3) {
                float r = imageData[idx] / 255.0f;
                float g = imageData[idx + 1] / 255.0f;
                float b = imageData[idx + 2] / 255.0f;
                float maxC = std::max({r, g, b});
                float minC = std::min({r, g, b});
                saturation = (maxC > 0.01f) ? (maxC - minC) / maxC : 0.0f;
            }

            float depth = baseDepth * 0.5f + lum * 0.15f + (1.0f - edge) * 0.15f + (1.0f - saturation) * 0.2f;
            data[y * w + x] = std::clamp(depth, 0.0f, 1.0f);
        }
    }

    int blurRadius = std::max(w, h) / 100;
    std::vector<float> blurred(w * h);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float sum = 0.0f;
            int count = 0;
            for (int dy = -blurRadius; dy <= blurRadius; dy++) {
                for (int dx = -blurRadius; dx <= blurRadius; dx++) {
                    int nx = std::clamp(x + dx, 0, w - 1);
                    int ny = std::clamp(y + dy, 0, h - 1);
                    sum += data[ny * w + nx];
                    count++;
                }
            }
            blurred[y * w + x] = sum / count;
        }
    }
    data = blurred;

    std::vector<unsigned char> texData(w * h);
    for (int i = 0; i < w * h; i++) {
        texData[i] = static_cast<unsigned char>(data[i] * 255.0f);
    }
    texture.loadFromMemory(texData.data(), w, h, 1);
}

float DepthMap::sampleDepth(float u, float v) const {
    if (data.empty()) return 0.5f;
    int x = std::clamp(static_cast<int>(u * width), 0, width - 1);
    int y = std::clamp(static_cast<int>(v * height), 0, height - 1);
    return data[y * width + x];
}

void DepthMap::destroy() {
    texture.destroy();
    data.clear();
}

bool Scene::loadImage(const std::string& path) {
    stbi_set_flip_vertically_on_load(false);
    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!data) {
        std::cerr << "Failed to load scene image: " << path << std::endl;
        return false;
    }

    imageWidth = w;
    imageHeight = h;
    rawChannels = ch;
    rawImageData.assign(data, data + w * h * ch);
    imagePath = path;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* flipped = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (flipped) {
        mainImage.loadFromMemory(flipped, w, h, ch);
        stbi_image_free(flipped);
    }

    stbi_image_free(data);
    loaded = true;

    generateDepthMap();
    generateLayers();

    return true;
}

void Scene::generateDepthMap() {
    if (rawImageData.empty()) return;
    depthMap.generate(rawImageData.data(), imageWidth, imageHeight, rawChannels);
}

void Scene::generateLayers() {
    layers.clear();

    layers.push_back({"Sky / Background", 0.7f, 1.0f, 0.85f, {}, true});
    layers.push_back({"Midground", 0.3f, 0.7f, 0.5f, {}, true});
    layers.push_back({"Foreground", 0.0f, 0.3f, 0.15f, {}, true});

    for (auto& layer : layers) {
        std::vector<unsigned char> layerData(imageWidth * imageHeight * 4);

        for (int y = 0; y < imageHeight; y++) {
            for (int x = 0; x < imageWidth; x++) {
                int srcIdx = (y * imageWidth + x) * rawChannels;
                int dstIdx = (y * imageWidth + x) * 4;
                float depth = depthMap.data[y * imageWidth + x];

                float alpha = 0.0f;
                if (depth >= layer.depthMin && depth <= layer.depthMax) {
                    float center = (layer.depthMin + layer.depthMax) * 0.5f;
                    float range = (layer.depthMax - layer.depthMin) * 0.5f;
                    float dist = std::abs(depth - center) / range;
                    alpha = 1.0f - std::pow(dist, 2.0f);
                    alpha = std::clamp(alpha, 0.0f, 1.0f);
                }

                for (int c = 0; c < std::min(rawChannels, 3); c++) {
                    layerData[dstIdx + c] = rawImageData[srcIdx + c];
                }
                if (rawChannels < 3) {
                    layerData[dstIdx + 1] = rawImageData[srcIdx];
                    layerData[dstIdx + 2] = rawImageData[srcIdx];
                }
                layerData[dstIdx + 3] = static_cast<unsigned char>(alpha * 255.0f);
            }
        }

        std::vector<unsigned char> flipped(imageWidth * imageHeight * 4);
        for (int y = 0; y < imageHeight; y++) {
            int srcRow = (imageHeight - 1 - y) * imageWidth * 4;
            int dstRow = y * imageWidth * 4;
            std::copy(layerData.data() + srcRow, layerData.data() + srcRow + imageWidth * 4,
                      flipped.data() + dstRow);
        }

        layer.texture.loadFromMemory(flipped.data(), imageWidth, imageHeight, 4);
    }
}

void Scene::destroy() {
    mainImage.destroy();
    depthMap.destroy();
    for (auto& layer : layers) {
        layer.texture.destroy();
    }
    layers.clear();
    rawImageData.clear();
    loaded = false;
}

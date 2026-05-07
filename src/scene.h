#pragma once
#include "gl_utils.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct DepthMap {
    std::vector<float> data;
    int width = 0, height = 0;
    Texture texture;

    void generate(const unsigned char* imageData, int w, int h, int channels);
    float sampleDepth(float u, float v) const;
    void destroy();
};

struct SceneLayer {
    std::string name;
    float depthMin = 0.0f;
    float depthMax = 1.0f;
    float depth = 0.5f;
    Texture texture;
    bool visible = true;
};

class Scene {
public:
    Texture mainImage;
    DepthMap depthMap;
    std::vector<SceneLayer> layers;
    int imageWidth = 0, imageHeight = 0;
    bool loaded = false;
    std::string imagePath;

    bool loadImage(const std::string& path);
    void generateDepthMap();
    void generateLayers();
    void destroy();

private:
    std::vector<unsigned char> rawImageData;
    int rawChannels = 0;
};

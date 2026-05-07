#pragma once
#include <string>
#include <cstdio>

class VideoExporter {
public:
    bool start(const std::string& outputPath, int width, int height, int fps);
    bool writeFrame(const unsigned char* rgbData, int width, int height);
    void finish();
    bool isExporting() const { return exporting; }
    int getFrameCount() const { return frameCount; }
    float getProgress(float duration, int fps) const;

private:
    FILE* ffmpegPipe = nullptr;
    bool exporting = false;
    int frameCount = 0;
    int targetWidth = 0;
    int targetHeight = 0;
};

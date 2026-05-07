#include "video_export.h"
#include <iostream>
#include <sstream>

bool VideoExporter::start(const std::string& outputPath, int width, int height, int fps) {
    if (exporting) {
        std::cerr << "Already exporting!" << std::endl;
        return false;
    }

    targetWidth = width;
    targetHeight = height;

    std::ostringstream cmd;
    cmd << "ffmpeg -y"
        << " -f rawvideo"
        << " -pixel_format rgb24"
        << " -video_size " << width << "x" << height
        << " -framerate " << fps
        << " -i -"
        << " -c:v libx264"
        << " -preset medium"
        << " -crf 18"
        << " -pix_fmt yuv420p"
        << " \"" << outputPath << "\""
        << " 2>/dev/null";

    ffmpegPipe = popen(cmd.str().c_str(), "w");
    if (!ffmpegPipe) {
        std::cerr << "Failed to open FFmpeg pipe" << std::endl;
        return false;
    }

    exporting = true;
    frameCount = 0;
    std::cout << "Started video export: " << outputPath << " (" << width << "x" << height << " @ " << fps << "fps)" << std::endl;
    return true;
}

bool VideoExporter::writeFrame(const unsigned char* rgbData, int width, int height) {
    if (!exporting || !ffmpegPipe) return false;

    size_t dataSize = static_cast<size_t>(width) * height * 3;
    size_t written = fwrite(rgbData, 1, dataSize, ffmpegPipe);

    if (written != dataSize) {
        std::cerr << "Failed to write frame data" << std::endl;
        return false;
    }

    frameCount++;
    return true;
}

void VideoExporter::finish() {
    if (ffmpegPipe) {
        pclose(ffmpegPipe);
        ffmpegPipe = nullptr;
    }
    exporting = false;
    std::cout << "Video export complete. Total frames: " << frameCount << std::endl;
}

float VideoExporter::getProgress(float duration, int fps) const {
    int totalFrames = static_cast<int>(duration * fps);
    if (totalFrames <= 0) return 1.0f;
    return static_cast<float>(frameCount) / totalFrames;
}

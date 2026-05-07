# Cinematic VFX Editor

A C++ real-time cinematic VFX editor that transforms static images into living, atmospheric scenes with professional visual effects, depth simulation, and cinematic camera animation.

## Features

### Scene Analysis
- **Image Loading**: Supports PNG, JPG, WEBP formats
- **Automatic Depth Map Generation**: AI-inspired depth estimation using edge detection, luminance, and saturation analysis
- **Layer Separation**: Automatic foreground/midground/background separation with depth-based masking

### VFX System (Drag & Drop)
- **Fire** - Dynamic flames with turbulence, inner/outer color gradient, and light emission
- **Rain** - Perspective-aware rainfall with wind direction control
- **Fog** - Volumetric ground fog that drifts naturally across terrain
- **Smoke** - Rising smoke with turbulence and expansion
- **Embers** - Floating hot particles with random drift
- **Snow** - Gentle snowfall with lateral sway
- **Lightning** - Scene-wide flash with configurable cooldown
- **Dust** - Subtle floating particles
- **God Rays** - Volumetric light shafts

### Smart Physics
- Smoke rises naturally
- Rain falls with perspective depth
- Fog flows across terrain
- Fire flickers dynamically
- Particles react to wind direction
- Each effect has configurable turbulence, wind, and intensity

### Lighting & Post-Processing
- **Bloom** with configurable threshold and intensity
- **HDR Tone Mapping** with exposure control
- **Color Grading** (contrast, saturation, tint)
- **Vignette** effect
- **Lightning Flash** integration (affects entire scene exposure)

### Camera System
- Pan, zoom, and rotation controls
- **Cinematic Drift** - Automatic slow camera movement for atmosphere
- **Camera Shake** - Configurable intensity and frequency
- **Parallax** - Depth-based layer movement for fake 3D effect

### Editor UI
- Professional dark cinematic theme (inspired by Unreal Engine / DaVinci Resolve)
- Dockable panel layout
- Effect palette with one-click effect creation
- Properties panel with per-effect controls
- Post-processing controls
- Camera controls
- Timeline with play/pause/reset
- Video export panel

### Video Export
- Export animated scenes to MP4 via FFmpeg
- Configurable resolution, FPS, and duration
- H.264 encoding with high quality settings

## Build Instructions

### Prerequisites
- CMake 3.16+
- C++17 compatible compiler (GCC 7+, Clang 5+)
- OpenGL 3.3+
- GLEW (`libglew-dev` on Ubuntu)
- FFmpeg (for video export)
- X11 development libraries (Linux)

### Ubuntu/Debian
```bash
sudo apt-get install -y cmake build-essential libglew-dev ffmpeg \
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
    libwayland-dev libxkbcommon-dev pkg-config
```

### Build
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run
```bash
./cinematic_vfx
```

Or with an image:
```bash
./cinematic_vfx /path/to/your/image.png
```

## Usage

1. **Load an Image**: Enter the image path in the Effects Palette panel and click "Load Image"
2. **Add Effects**: Click effect buttons (Fire, Rain, Fog, etc.) to add them to the scene
3. **Configure Effects**: Select an effect in the list, then adjust properties in the Properties panel
4. **Adjust Camera**: Use the Camera tab to control drift, shake, and zoom
5. **Post-Processing**: Use the Post FX tab to adjust bloom, color grading, and vignette
6. **Export**: Use the Export tab to render an animated video

### Controls
- **Mouse Wheel**: Zoom in/out on viewport
- **Middle Mouse Drag**: Pan the viewport
- **Right Click**: Context interaction on viewport
- **Escape**: Exit application

## Architecture

```
src/
├── main.cpp              Entry point
├── application.h/cpp     GLFW window, main loop, system coordination
├── gl_utils.h/cpp        Shader, Texture, Framebuffer, QuadRenderer
├── shaders.h             All GLSL shaders as embedded strings
├── scene.h/cpp           Image loading, depth map, layer separation
├── vfx_system.h/cpp      Particle system, all VFX effect types
├── post_process.h/cpp    Bloom, tone mapping, color grading, vignette
├── camera.h/cpp          Camera with drift, shake, parallax
├── editor.h/cpp          Full ImGui editor UI
├── video_export.h/cpp    FFmpeg pipe-based video export
└── stb_impl.cpp          stb_image implementations
```

### Dependencies (auto-downloaded via CMake FetchContent)
- **GLFW 3.3.8** - Windowing and input
- **GLM 0.9.9.8** - Math library
- **Dear ImGui v1.89.9** - Editor UI
- **stb** - Image loading (stb_image)

### System Dependencies
- **GLEW** - OpenGL extension loading
- **OpenGL 3.3** - Rendering
- **FFmpeg** - Video encoding

## Tech Stack

| Purpose | Technology |
|---------|-----------|
| Language | C++17 |
| Build System | CMake |
| Rendering | OpenGL 3.3 Core Profile |
| UI | Dear ImGui (Docking) |
| Math | GLM |
| Image I/O | stb_image |
| Video Export | FFmpeg (pipe) |
| Windowing | GLFW |

## License

This project is provided as-is for educational and creative purposes.

# Shader Toy

A C++17 OpenGL shader playground inspired by Shadertoy. The application opens a GLFW window, renders a fullscreen quad, passes time, resolution, and mouse input to GLSL, and supports live shader reloading.

## Features

- OpenGL 4.1 Core rendering through GLFW and GLAD
- Fullscreen quad for fragment-shader experiments
- Shadertoy-style uniforms: `iResolution`, `iTime`, and `iMouse`
- Shadertoy-style `mainImage(out vec4, in vec2)` support
- Four generated texture channels:
  - `iChannel0`: buffer output when `shaders/1.frag` is enabled, otherwise generated skin texture
  - `iChannel1`: generated noise texture
  - `iChannel2`: generated stone texture
  - `iChannel3`: generated cubemap environment
- Optional two-pass rendering through `shaders/1.frag`
- Hot reload for `shaders/toy.frag` and `shaders/1.frag`
- Failed shader reloads keep the last valid shader active

## Requirements

- C++17 compiler
- CMake 3.10 or newer
- GLFW 3
- OpenGL 4.1 capable graphics driver

On macOS with Homebrew:

```bash
brew install cmake glfw
```

GLAD is included in the repository under `src/` and `include/`.

## Build

From the project root:

```bash
cmake -S . -B build
cmake --build build -j4
```

The CMake configuration copies the `shaders/` directory into `build/shaders` after building.

## Run

Run from the build directory because the executable uses paths relative to it:

```bash
cd build
./ShaderToy
```

The application stays open until the window is closed. Shader changes are detected while it is running.

## Shader Workflow

### Single pass

Edit `shaders/toy.frag`. Without `shaders/1.frag`, it is rendered directly to the screen.

You can write a regular GLSL entry point:

```glsl
void main()
{
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
```

You can also paste a Shadertoy-style shader:

```glsl
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    fragColor = vec4(uv, 0.0, 1.0);
}
```

The loader automatically adds the GLSL version, common uniforms, channel declarations, and a `main()` wrapper when `mainImage` is present.

### Optional buffer pass

Create `shaders/1.frag` to enable a two-pass pipeline:

1. `1.frag` renders to an offscreen framebuffer.
2. `toy.frag` renders to the window and can sample the buffer through `iChannel0`.

Example:

```glsl
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    fragColor = vec4(uv.x, uv.y, 0.2, 1.0);
}
```

When `1.frag` is created, modified, or removed, the application updates the active pipeline automatically.

## Available Uniforms

```glsl
uniform vec2 iResolution;
uniform float iTime;
uniform vec2 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform samplerCube iChannel3;
```

Mouse coordinates are supplied in window pixels. `iTime` is elapsed seconds since startup.

## Project Structure

```text
Shader-Toy/
├── CMakeLists.txt
├── include/
│   ├── glad/
│   └── KHR/
├── shaders/
│   ├── default.vert
│   ├── default.frag
│   ├── toy.frag
│   └── 1.frag          # optional buffer pass
└── src/
    ├── main.cpp
    ├── Window.*
    ├── Quad.*
    ├── ShaderProgram.*
    ├── FileWatcher.*
    ├── TextureGenerator.*
    └── glad.c
```

## Troubleshooting

### Shader files are not found

Run the executable from `build/`:

```bash
cd build
./ShaderToy
```

### Shader compilation fails

Check the terminal output. The previous valid shader remains active after a failed hot reload, so fix the shader and save it again.

### No visible texture input

`iChannel0` is the offscreen buffer only when `shaders/1.frag` exists. Otherwise use `iChannel1`, `iChannel2`, or `iChannel3` for the generated texture inputs.

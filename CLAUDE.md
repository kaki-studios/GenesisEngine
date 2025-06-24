# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

Build the project:
```bash
cmake -S . -B build
cd build && make -j 8
```

Run the demo:
```bash
./build/bin/EngineDemo
```

Clean build:
```bash
rm -rf build/
```

## Architecture Overview

This is a C++ 3D game engine built with modern graphics APIs. The engine uses:

- **bgfx** - Cross-platform rendering backend (supports Vulkan, OpenGL, Direct3D, Metal)
- **GLFW** - Cross-platform windowing and input handling
- **GLM** - Mathematics library for graphics programming

### Core Components

**Engine Library (`src/`)**
- Built as a static library containing all engine functionality
- Organized into modular subsystems: core, platform, rendering, physics

**Platform Layer (`src/platform/`)**
- `GLFWIntegration.h/cpp` - Platform abstraction for native window handles
- Supports X11 and Wayland on Linux, with automatic detection
- Cross-platform support for Windows, macOS, and Linux

**Core Application (`src/core/`)**
- `App` class - Main application lifecycle management
- Handles GLFW initialization, window creation, and main loop
- Integrates bgfx rendering initialization

**Demo Application (`demo/`)**
- Simple demonstration of engine usage
- Creates 800x600 window with basic bgfx rendering

### Platform Detection

The engine has sophisticated platform detection in `src/include/Platform.h`:
- Automatic Wayland vs X11 detection on Linux
- Compile-time platform macros for conditional compilation
- Runtime environment detection for display server selection

### Build System

- Uses CMake with modern target-based configuration
- Automatically detects Wayland session and configures GLFW accordingly
- Links required system libraries (Wayland client libraries on Linux)
- Organizes source files with glob patterns for easy maintenance

### Key Technical Details

- bgfx requires `renderFrame()` call before initialization to prevent separate render thread creation
- Native window handle extraction varies by platform (X11 Window vs Wayland surface)
- Engine uses bgfx's platform data interface for proper graphics context setup

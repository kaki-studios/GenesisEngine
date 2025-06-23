#pragma once
#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

namespace Engine {
struct NativeWindowHandle {
  void *display; // for x11/wayland
  void *window;  // common for all platforms
};

NativeWindowHandle getNativeWindowHandle(GLFWwindow *window);
void initBGFX(GLFWwindow *window,
              bgfx::RendererType::Enum type = bgfx::RendererType::Count);
} // namespace Engine

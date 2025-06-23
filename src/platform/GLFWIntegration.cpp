#include "GLFWIntegration.h"
#include "Platform.h"
#include "bgfx/bgfx.h"
#include <cstdint>
#include <iostream>
// #include "include/Platform.h"
#include <GLFW/glfw3.h>
#include <cstdlib>

#if PLATFORM_LINUX
#if PLATFORM_WAYLAND
#include <wayland-client.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#else
#define GLFW_EXPOSE_NATIVE_X11
#endif
#elif PLATFORM_MACOS
#define GLFW_EXPOSE_NATIVE_COCOA
#elif PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3native.h>

namespace Engine {
NativeWindowHandle getNativeWindowHandle(GLFWwindow *window) {
  NativeWindowHandle handle = {};
#if PLATFORM_LINUX
#if PLATFORM_WAYLAND
  std::cerr << "wayland" << std::endl;
  handle.display = glfwGetWaylandDisplay();
  handle.window = glfwGetWaylandWindow(window);
#else

  std::cerr << "x11" << std::endl;
  handle.display = glfwGetX11Display();
  handle.window = (void *)(uintptr_t)glfwGetX11Window(window);
#endif
#elif PLATFORM_MACOS

  std::cerr << "macos" << std::endl;
  handle.window = glfwGetCocoaWindow(window);
#elif PLATFORM_WINDOWS

  std::cerr << "win32" << std::endl;
  handle.window = glfwGetWin32Window(window);

#else
  std::cerr << "no platform" << std::endl;
#endif

  // After getting native handles
  printf("Display: %p, Window: %p\n", handle.display, handle.window);
  if (!handle.window) {
    fprintf(stderr, "Failed to get native window handle!\n");
    abort();
  }

  return handle;
}

void initBGFX(GLFWwindow *window, bgfx::RendererType::Enum type) {
  NativeWindowHandle handle = getNativeWindowHandle(window);
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  bgfx::Init init;

#if BX_PLATFORM_LINUX
#if PLATFORM_WAYLAND
  init.platformData.ndt = handle.display;
  init.platformData.nwh = handle.window;
#else
  init.platformData.ndt = handle.display;
  init.platformData.nwh = (void *)(uintptr_t)handle.window;
#endif
#elif BX_PLATFORM_OSX
  init.platformData.nwh = handle.window;
#elif BX_PLATFORM_WINDOWS
  init.platformData.nwh = handle.window;
#endif
  // Verify handles
  if (!init.platformData.nwh) {
    fprintf(stderr, "Invalid native window handle!\n");
    abort();
  }

  init.type = type;
  init.debug = true;
  init.resolution.width = (uint32_t)width;
  init.resolution.height = (uint32_t)height;
  init.resolution.reset = BGFX_RESET_VSYNC;

  if (!bgfx::init(init)) {
    std::cerr << "Failed to initialize bgfx" << std::endl;
    std::exit(1);
  }
  bgfx::setViewClear(0, BGFX_CLEAR_COLOR);
  bgfx::setViewRect(0, 0, 0, bgfx::BackbufferRatio::Equal);
}
} // namespace Engine

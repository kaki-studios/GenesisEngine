#include "GLFWIntegration.h"
#include "Platform.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include <cstdint>
#include <iostream>
// #include "include/Platform.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <sys/types.h>

#if PLATFORM_LINUX
// Include both X11 and Wayland support for runtime detection
#include <wayland-client.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_EGL
#define GLFW_EXPOSE_NATIVE_GLX
#elif PLATFORM_MACOS
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#elif PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3native.h>

namespace Engine {

NativeWindowHandle getNativeWindowHandle(GLFWwindow *window) {

  NativeWindowHandle handle = {};
#if PLATFORM_LINUX
  // Runtime detection of display server
  if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
    std::cerr << "wayland" << std::endl;
    handle.display = glfwGetWaylandDisplay();
    handle.window = glfwGetWaylandWindow(window);
  } else {
    std::cerr << "x11" << std::endl;
    handle.display = glfwGetX11Display();
    handle.window = (void *)(uintptr_t)glfwGetX11Window(window);
  }
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

void initBGFX(GLFWwindow *window) {
  NativeWindowHandle handle = getNativeWindowHandle(window);
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  bgfx::PlatformData pd;

#if BX_PLATFORM_LINUX
  // Runtime detection of display server for bgfx
  if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
    pd.type = bgfx::NativeWindowHandleType::Wayland;
    pd.ndt = handle.display;
    pd.nwh = handle.window; // Direct pointer for Wayland surface
    std::cerr << "Setting bgfx platform type to Wayland" << std::endl;
    std::cerr << "pd.type = " << (int)pd.type << std::endl;
    std::cerr << "pd.ndt = " << pd.ndt << std::endl;
    std::cerr << "pd.nwh = " << pd.nwh << std::endl;
  } else {
    pd.type = bgfx::NativeWindowHandleType::Default; // X11
    pd.ndt = handle.display;
    pd.nwh = (void *)(uintptr_t)handle.window; // X11 needs uintptr_t cast
    std::cerr << "Setting bgfx platform type to X11/Default" << std::endl;
  }
#elif BX_PLATFORM_OSX
  pd.nwh = handle.window;
#elif BX_PLATFORM_WINDOWS
  pd.nwh = handle.window;
#endif
  // Verify handles
  if (!pd.nwh) {
    fprintf(stderr, "Invalid native window handle!\n");
    abort();
  }
  pd.backBuffer = nullptr;
  pd.backBufferDS = nullptr;
  pd.context = nullptr;
  bgfx::setPlatformData(pd);
  bgfx::Init init;
  init.platformData = pd;
  if (glfwVulkanSupported()) {
    init.type = bgfx::RendererType::Vulkan;
  }

  // init.type = type;
  // init.debug = true;
  init.resolution.width = (uint32_t)width;
  init.resolution.height = (uint32_t)height;
  init.resolution.reset = BGFX_RESET_VSYNC;

  if (!bgfx::init(init)) {
    std::cerr << "Failed to initialize bgfx" << std::endl;
    std::exit(1);
  }
  bgfx::setViewClear(0, BGFX_CLEAR_COLOR);
  bgfx::setViewRect(0, 0, 0, bgfx::BackbufferRatio::Equal);
  std::cout << "Successfully initialized bgfx with "
            << bgfx::getRendererName(bgfx::getRendererType()) << std::endl;
}
} // namespace Engine

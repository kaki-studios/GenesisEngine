#include "integration.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>
#include <X11/Xlib.h>
#include <cstdint>
#include <iostream>

namespace Integration {
void IntegrateToBGFX(SDL_Window *window) {
  bgfx::PlatformData pd;
  SDL_PropertiesID props = SDL_GetWindowProperties(window);
#if defined(SDL_PLATFORM_WIN32)
  HWND hwnd = (HWND)SDL_GetPointerProperty(
      props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
  if (hwnd) {
    pd.nwh = hwnd;
  }
#elif defined(SDL_PLATFORM_MACOS)
  NSWindow *nswindow = (__bridge NSWindow *)SDL_GetPointerProperty(
      props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
  if (nswindow) {
    pd.nwh = nswindow;
  }
#elif defined(SDL_PLATFORM_LINUX)
  if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
    std::cout << "using x11" << std::endl;
    Display *xdisplay = (Display *)SDL_GetPointerProperty(
        SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER,
        NULL);
    Window xwindow = (Window)SDL_GetNumberProperty(
        SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
    if (xdisplay && xwindow) {
      pd.nwh = (void *)(uintptr_t)xwindow;
      pd.ndt = xdisplay;
      pd.type = bgfx::NativeWindowHandleType::Default;
    }
  } else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
    std::cout << "using wayland" << std::endl;
    struct wl_display *display = (struct wl_display *)SDL_GetPointerProperty(
        SDL_GetWindowProperties(window),
        SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
    struct wl_surface *surface = (struct wl_surface *)SDL_GetPointerProperty(
        SDL_GetWindowProperties(window),
        SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
    if (display && surface) {
      pd.nwh = surface;
      pd.ndt = display;
      pd.type = bgfx::NativeWindowHandleType::Wayland;

    } else {
      std::cout << "failed to get wayland window or surface\n";
      abort();
    }
  }
#endif

  pd.backBuffer = nullptr;
  pd.backBufferDS = nullptr;
  pd.context = nullptr;

  bgfx::setPlatformData(pd);
  bgfx::Init init;
  init.debug = false;
  init.platformData = pd;
  int width, height;
  SDL_GetWindowSize(window, &width, &height);
  init.resolution.width = (uint32_t)width;
  init.resolution.height = (uint32_t)height;
  init.resolution.reset = BGFX_RESET_VSYNC;
#ifndef NDEBUG
  init.type = bgfx::RendererType::OpenGL;
  std::cout << "debug build" << std::endl;
#endif

  if (!bgfx::init(init)) {
    std::cerr << "Failed to initialize bgfx" << std::endl;
    std::exit(1);
  }
  std::cout << "Successfully initialized bgfx with "
            << bgfx::getRendererName(bgfx::getRendererType()) << std::endl;
}
} // namespace Integration

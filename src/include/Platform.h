#pragma once

// Platform detection using predefined macros
#ifdef _WIN32
/* Windows x64/x86 */
#define PLATFORM_WINDOWS 1
#ifdef _WIN64
#define PLATFORM_WINDOWS_64 1
#else
#define PLATFORM_WINDOWS_32 1
#endif
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
/* Apple platforms */
#define PLATFORM_APPLE 1
#if TARGET_IPHONE_SIMULATOR
#define PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define PLATFORM_IOS 1
#elif TARGET_OS_MAC
#define PLATFORM_MACOS 1
#endif
#elif defined(__ANDROID__)
#define PLATFORM_ANDROID 1
#elif defined(__linux__)
/* Linux */
#define PLATFORM_LINUX 1
#if defined(__WAYLAND__)
#define PLATFORM_WAYLAND 1
#include <wayland-client.h>
#elif !defined(__WAYLAND__)
#ifdef __X11__
#define PLATFORM_X11 1
#else
// Runtime detection - both X11 and Wayland support compiled in
#define PLATFORM_X11 1
#define PLATFORM_WAYLAND 1
#include <cstdlib>
#include <string.h>
static inline bool isWaylandSession() {
  const char *sessionType = getenv("XDG_SESSION_TYPE");
  const char *waylandDisplay = getenv("WAYLAND_DISPLAY");
  return (sessionType && strstr(sessionType, "wayland")) || waylandDisplay;
}
#endif

#endif
#else
#error "Unknown platform!"
#endif

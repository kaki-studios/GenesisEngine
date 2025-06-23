#pragma once

// Platform detection using predefined macros
#include <string.h>
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
#include <wayland-client.h>
#if defined(__WAYLAND__)
#define PLATFORM_WAYLAND 1

#elif !defined(PLATFORM_WAYLAND)
#ifdef __X11__
#define PLATFORM_X11 1
#else
// Auto-detect based on environment
#include <stdlib.h>
static const bool waylandSession =
    strstr(getenv("XDG_SESSION_TYPE"), "wayland");
#if waylandSession
#define PLATFORM_WAYLAND 1
#define __WAYLAND__ 1
#else
#define PLATFORM_X11 1
#define __X11__ 1
#endif
#endif

#endif
#else
#error "Unknown platform!"
#endif

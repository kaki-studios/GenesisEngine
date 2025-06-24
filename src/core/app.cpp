#include "app.h"
#include "Platform.h"
#include "bgfx/defines.h"
#include "platform/GLFWIntegration.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
// #include <thread>

#include <GLFW/glfw3.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

static void glfw_errorCallback(int error, const char *description) {
  fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

static void glfw_keyCallback(GLFWwindow *window, int key, int scancode,
                             int action, int mods) {
  if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
    std::cerr << "should close" << std::endl;
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

static void glfw_closeCallback(GLFWwindow *window) {
  std::cerr << "window closing" << std::endl;
  // glfwSetWindowShouldClose(window, GLFW_TRUE);
  std::cerr << "called window close" << std::endl;
}

// creates an app
App::App(int width, int height) {
#if PLATFORM_WAYLAND
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
  glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
  glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
#endif
  glfwSetErrorCallback(glfw_errorCallback);
  if (!glfwInit()) {
    // fatal error
    std::cerr << "Couldn't initialize GLFW" << std::endl;
    std::exit(1);
  }
  std::cerr << (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) << std::endl;
  // we will provide our own api (bgfx)
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  GLFWwindow *window = glfwCreateWindow(width, height, "TEST TITLE",
                                        glfwGetPrimaryMonitor(), nullptr);

  if (!window) {
    glfwTerminate();
    std::cerr << "Couldn't initialize window" << std::endl;
    std::exit(1);
  }
  glfwSetKeyCallback(window, glfw_keyCallback);
  glfwSetWindowCloseCallback(window, glfw_closeCallback);
  std::cout << "rendering frame" << std::endl;

  // renders frame before init so bgfx doesn't create a seperate render thread
  // since the window creation and rendering must be on the same thread (on most
  // rendering apis) TODO a seperate render thread
  bgfx::renderFrame();
  std::cout << "initializing bgfx" << std::endl;
  Engine::initBGFX(window);
}

App::~App() {
  bgfx::shutdown();
  if (window)
    glfwDestroyWindow(window);
  glfwTerminate();
}

void App::Start() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff);
    bgfx::touch(0);

    bgfx::frame();
  }
  std::cerr << "broke" << std::endl;
}

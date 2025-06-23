#include "app.h"
#include "bgfx/defines.h"
#include "platform/GLFWIntegration.h"
#include <cstdlib>
#include <iostream>

#include <GLFW/glfw3.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

// creates an app
App::App(int width, int height) {
  if (!glfwInit()) {
    // fatal error
    std::cerr << "Couldn't initialize GLFW" << std::endl;
    std::exit(1);
  }
  // we will provide our own api (bgfx)
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  GLFWwindow *window =
      glfwCreateWindow(width, height, "TEST TITLE", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    std::cerr << "Couldn't initialize window" << std::endl;
    std::exit(1);
  }
  std::cout << "rendering frame" << std::endl;

  // renders frame before init so bgfx doesn't create a seperate render thread
  // since the window creation and rendering must be on the same thread (on most
  // rendering apis) TODO a seperate render thread would be better
  bgfx::renderFrame();
  std::cout << "initializing bgfx" << std::endl;
  Engine::initBGFX(window, bgfx::RendererType::Count);
}

App::~App() {
  bgfx::shutdown();
  glfwTerminate();
}

void App::Start() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff);
    bgfx::touch(0);
    bgfx::frame();
  }
}

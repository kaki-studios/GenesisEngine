#include "app.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>
// #include <imgui.h>
#include <iostream>
#include <platform/integration.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

// creates an app
App::App(int width, int height) {
  coordinator.Init();
  quit = false;
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "Couldn't initialize SDL" << std::endl;
    abort();
  }

  SDL_Window *window =
      SDL_CreateWindow("TEST TITLE", width, height,
                       SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);
  // ImGuiContext *ctx = ImGui::CreateContext();
  // std::cout << "context is " << ctx << std::endl;

  SDL_SetWindowRelativeMouseMode(window, true);
  SDL_SetWindowFullscreen(window, false);
  this->window = window;
  if (!window) {
    SDL_Quit();
    std::cerr << "Couldn't initialize window" << std::endl;
    abort();
  }
  // renders frame before init so bgfx doesn't create a seperate render thread
  // since the window creation and rendering must be on the same thread (on most
  // rendering apis) TODO a seperate render thread
  bgfx::renderFrame();
  std::cout << "initializing bgfx" << std::endl;
  Integration::IntegrateToBGFX(window);
}

App::~App() {

  coordinator.FreeSystems();
  bgfx::shutdown();
  if (window)
    SDL_DestroyWindow(window);
  SDL_Quit();
}

SDL_Window *App::GetWindow() { return window; }

bool App::ShouldClose() { return quit; }
void App::Update() {
  inputState.mouseDeltaX = 0;
  inputState.mouseDeltaY = 0;
  inputState.scrollX = 0;
  inputState.scrollY = 0;

  while (SDL_PollEvent(&this->currentEvent)) {
    switch (currentEvent.type) {
    case SDL_EVENT_QUIT: {

      std::cerr << "quitting" << std::endl;
      quit = true;
      break;
    }
    case SDL_EVENT_WINDOW_RESIZED: {

      int newWidth = currentEvent.window.data1;
      int newHeight = currentEvent.window.data2;

      // Update bgfx backbuffer size
      bgfx::reset((uint32_t)newWidth, (uint32_t)newHeight, BGFX_RESET_VSYNC);
      break;
    }
    case SDL_EVENT_KEY_DOWN: {
      inputState.keysDown[currentEvent.key.key] = true;
      break;
    }
    case SDL_EVENT_KEY_UP: {
      inputState.keysDown[currentEvent.key.key] = false;
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      inputState.mouseButtonsDown[currentEvent.button.button] = true;
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      inputState.mouseButtonsDown[currentEvent.button.button] = false;
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      inputState.mouseX = currentEvent.motion.x;
      inputState.mouseY = currentEvent.motion.y;
      inputState.mouseDeltaX = currentEvent.motion.xrel;
      inputState.mouseDeltaY = currentEvent.motion.yrel;
      break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
      inputState.scrollX = currentEvent.wheel.x;
      inputState.scrollY = currentEvent.wheel.y;
      break;
    }
    };
  }
}

bool App::GetWindowDims(int *width, int *height) {
  return SDL_GetWindowSize(this->window, width, height);
}

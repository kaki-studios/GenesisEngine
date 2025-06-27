#pragma once
#include "../ecs/coordinator.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

class App {

public:
  SDL_Window *window;
  SDL_Event currentEvent;
  bool quit;
  App(int width, int height);
  bool ShouldClose();
  void Update();
  bool GetWindowDims(int *width, int *height);
  ECS::Coordinator coordinator;
  ~App();
};

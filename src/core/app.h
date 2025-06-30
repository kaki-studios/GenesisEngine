#pragma once
#include "../ecs/coordinator.h"
#include "input.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

class App {
public:
  const InputState &GetInputState() const { return inputState; };
  App(int width, int height);
  bool ShouldClose();
  void Update();
  bool GetWindowDims(int *width, int *height);
  ECS::Coordinator coordinator;
  ~App();

private:
  SDL_Window *window;
  SDL_Event currentEvent;
  bool quit;
  InputState inputState;
};

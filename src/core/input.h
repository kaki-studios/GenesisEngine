#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_stdinc.h>
#include <unordered_map>

struct InputState {
  std::unordered_map<SDL_Keycode, bool> keysDown;
  std::unordered_map<Uint8, bool> mouseButtonsDown;
  int mouseX = 0, mouseY = 0;
  int mouseDeltaX = 0, mouseDeltaY = 0;
  int scrollX = 0, scrollY = 0;
};

# High Level TODOs

- [ ] Game loop (APIs etc.)
- [ ] Simple ECS/entity system
- [ ] Renderer
- [ ] Physics Engine (XPBD)

## Low Level TODOs
- [ ] Window closing
- [ ] Determine wayland/x11 at comptime

## Bugs
- [ ] glfw window doesn't close even when should close callback is called
- [ ] works in wayland only if "-DCMAKE_BUILD_TYPE=Debug" is not set (FIX: set -DCMAKE_BUILD_TYPE=RelWithDebInfo)
- [ ] works in x11 only if "-DCMAKE_BUILD_TYPE=Debug" is set

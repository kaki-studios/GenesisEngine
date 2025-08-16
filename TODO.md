# High Level TODOs

- [x] Game loop (APIs etc.) (kind of)
- [x] ECS
- [X] Renderer
- [ ] ImGui integration
- [ ] Physics Engine (XPBD)

## Low Level TODOs
- [ ] Camera controls and debugging
- [ ] Collisions working properly
    - [ ] Study XPBD more since the collision resolution is wrong.

## Architecture
- [X] Replace SAT collision logic with gjk + epa collision logic

## Bugs
- [ ] works in wayland only if "-DCMAKE_BUILD_TYPE=Debug" is not set (FIX: set -DCMAKE_BUILD_TYPE=RelWithDebInfo)
- [ ] works in x11 only if "-DCMAKE_BUILD_TYPE=Debug" is set

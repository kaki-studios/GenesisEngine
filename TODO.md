# High Level TODOs

- [x] Game loop (APIs etc.) (kind of)
- [x] ECS
- [ ] Renderer
    - [X] Minimal cuboid renderer
    - [ ] Mesh renderer
    - [ ] Proper shading
- [ ] ImGui integration
- [ ] Physics Engine (XPBD)

## Low Level TODOs
- [X] Camera controls and debugging
- [X] Collisions working properly
    - [ ] Study XPBD more since the collision resolution is wrong.
        - [X] Incorrect contact manifolds, points shifted by a constant amount per collision IMPORTANT
        - [ ] Warm starting (store lagrangeMultiplier across frames)
        - [X] Solver injects energy and other bugs??
            - [X] Collision detection: wrong contact point locations (check sutherland-hogdman & epa)
            - [X] Solver seems to increase penetration at first before seperating the bodies (in 1 frame)??
            - [X] TODO: restitution
            - [X] Collision detection bug: sutherland-hogdman gives penetration, but after integrating bodies, the penetration goes *down* after recomputing the penetration (0.06 to 0.05) even though it should go up!!
    - [X] Recompute collision penetration per substep
        - [X] In narrowphase: transform collsion points to bodies' local space
        - [X] In solver: transform them back to global space and recompute the penetration

## Architecture
- [X] Replace SAT collision logic with gjk + epa collision logic

## Bugs
- [ ] Two colliding cuboid rigidbodies produce errors in sutherland-hogdman (degenerate) and gjk
- [ ] Angular velocity is not properly integrated!! (probably constraints interfering)
- [X] works in wayland only if "-DCMAKE_BUILD_TYPE=Debug" is not set (FIXED: use opengl in debug builds)
- [ ] works in x11 only if "-DCMAKE_BUILD_TYPE=Debug" is set

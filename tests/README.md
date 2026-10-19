# Tests

`test_contacts.cpp` — headless checks for the XPBD contact pipeline
(GJK → EPA → clipping → position solve → velocity solve).

Build and run (needs glm; no SDL/bgfx required):

```bash
clang++ -std=c++17 -O1 -I/path/to/glm -Isrc -Isrc/include -Isrc/ecs \
  tests/test_contacts.cpp \
  src/physics/collision/gjk.cpp src/physics/collision/epa.cpp \
  src/physics/collision/clipping.cpp src/physics/collision/collision.cpp \
  src/physics/collision/cuboid_collider.cpp src/physics/cuboid/collision.cpp \
  src/ecs/coordinator.cpp src/ecs/entity_manager.cpp \
  src/ecs/system_manager.cpp -o test_contacts
./test_contacts
```

Checks: resting box does not sink, a rotated box is pushed out rather than
sheared, a deeply overlapping box settles instead of being launched, contact
normals point A→B with positive penetration, a two-box stack holds, and
friction arrests a sliding box.

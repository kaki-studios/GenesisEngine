#include "../core/app.h"
#include <SDL3/SDL_events.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

// free move camera
struct Camera {
  // included in transform
  //  glm::vec3 position;
  //  glm::quat rotation;
  float fov;
  float moveSpeed;
  float nearPlane;
  float farPlane;

  float pitch;
  float yaw;
};

class CameraSystem : public ECS::System {
public:
  void Update(float dt);
  void Init(App *app);

private:
  App *app;
};

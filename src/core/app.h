#include <GLFW/glfw3.h>

class App {
  GLFWwindow *window;

public:
  App(int width, int height);
  void Start();
  ~App();
};

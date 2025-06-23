#include <Engine.h>
#include <iostream>

int main(void) {
  std::cout << "Hello, World!" << std::endl;
  App app(800, 600);
  std::cout << "app started" << std::endl;
  app.Start();
  std::cout << "Out" << std::endl;

  return 0;
}

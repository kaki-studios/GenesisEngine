#include <Engine.h>
#include <iostream>
#include <ostream>

int main(void) {
  std::cout << "starting engine demo" << std::endl;
  App app(800, 600);
  std::cout << "app started" << std::endl;
  app.Start();
  std::cout << "Out" << std::endl;

  return 0;
}

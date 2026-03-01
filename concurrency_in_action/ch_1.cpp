#include <iostream>
#include <thread>

void print_hello() { std::cout << "Hello Concurrent World!\n"; }

int main() {
  std::thread obj(print_hello);
  print_hello();
  obj.join();
  return 0;
}

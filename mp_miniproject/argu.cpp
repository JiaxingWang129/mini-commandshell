#include <stdio.h>
#include <iostream>

int main(int argc, char ** argv){
  for(int i = 1; i < argc; i++){
    std::cout << "arguments[" << i << "]: " << argv[i] << std::endl;
  }
  return 0;
}

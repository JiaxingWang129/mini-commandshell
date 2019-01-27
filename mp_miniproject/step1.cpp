#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <string>
void skipSpace(const char ** s) {
  while (isspace(**s)) {
    *s = *s + 1;
  }
}
//bool checkfile(std::string c) {}

std::string parsefilename(std::string c) {
  std::size_t found = c.find_first_of("/");
  return (c.substr(found + 1));
}
using namespace std;
int main() {
  char promp[10000];
  getcwd(promp, sizeof(promp));
  std::cout << "myShell:~" << promp << "$";
  std::string commandline;
  while (getline(std::cin, commandline)) {
    //meet eof exit sucess
    if (std::cin.eof()) {
      exit(EXIT_SUCCESS);
    }
    //run a file
    if (!commandline.compare(0, 2, "./")) {
      std::string filename = parsefilename(commandline);
      //check whether this file exists or not----this can be implement in the next step this step can use system call to judge the error if there is no such file the exit value would reflect
      //if (checkfile(filename) == true) {
      if (true) {
        std::string excuteRun = "g++ ";
        excuteRun = excuteRun + "-o a.out " + filename;
        const char * path = excuteRun.c_str();
        //checking whether the processor is avliable
        if (system(NULL)) {
          puts("the processor is avaliable");
        }
        else {
          exit(EXIT_FAILURE);
        }
        system(path);
        int exitstatus;

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "../inc/lexer.hpp"
#include "../inc/parser.hpp"
#include "../inc/assembler.hpp"
#include "../inc/linker.hpp"
#include "../inc/emulator.hpp"

using namespace std;

int main(int argc, char *argv[])
{

  std::vector<string> args;
  args.assign(argv, argv + argc);
  string inputFile = args[1];
  
  FILE *myfile = fopen(inputFile.c_str(), "r");
  if (!myfile)
  {
    cout << "I can't open file!" << endl;
    return -1;
  }

  Emulator *emulator = new Emulator();
  emulator->readFile(inputFile);
  
  emulator->emulate();
  emulator->printOutput("emulatorOuput");
  
  return 0;
}

#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>

using namespace std;

class Emulator
{

public:
  Emulator() {}
  void readFile(string filename);
  void emulate();
  void printOutput(string outFile);
  string formatLittleEndian(int val);
  int littleEndianHexToInt(string input);
  string formatOperandFourBytes(string value);
  string decimalToHex(int decimalNumber);
  void halt();
  void interrupt();
  void callMod0(int a, int b, int disp);
  void callMod1(int a, int b, int disp);
  void jumpDirect(int a, int disp);
  void jump(int a, int disp);
  void jumpEQ(int a,int b,int c,int disp);
  void jumpNEQ(int a,int b,int c,int disp);
  void jumpGT(int a,int b,int c,int disp);
  void xchg(int a, int b);
  void add(int a, int b, int c);
  void sub(int a, int b, int c);
  void mul(int a, int b, int c);
  void div(int a, int b, int c);
  void notInstr(int a, int b);
  void andInstr(int a, int b, int c);
  void orInstr(int a, int b, int c);
  void xorInstr(int a, int b, int c);
  void shiftLeft(int a, int b, int c);
  void shiftRight(int a, int b, int c);
  void push(int c); //c se pushuje
  void csrrd(int a, int b);
  void csrwr(int a, int b);
  void loadInReg(int a,int b,int disp);
  void load(int a, int b, int c, int disp);
  void loadCsr(int a, int b, int c, int disp);
  int pop(int a, int b);
  int popCsr(int a, int b,int disp);
  void storeMod0(int a,int b,int c, int disp);
  void storeMod2(int a,int b,int c, int disp);

 /* void addShl(int a,int b, int c, int disp);
  void addShr(int a,int b, int c, int disp);
  void subShl(int a,int b, int c, int disp);
  void subShr(int a,int b, int c, int disp); */
  string formatRegisterValue(unsigned int value);

  vector<string> split(const string &str, char delimiter);

private:
  unsigned int regs[16];
  unsigned int csr[3];
  const int sp = 14;
  const int pc = 15;
  const int status = 0;
  const int handler = 1;
  const int cause = 2;
  bool endProg=false;
  map<unsigned int, string> memory;

};

#endif
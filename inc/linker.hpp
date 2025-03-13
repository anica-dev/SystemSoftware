#ifndef LINKER_H_
#define LINKER_H_

#include "../inc/stuctures.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>

using namespace std;

class Linker{

  public:

  Linker() {}
  void readFile(string filename);
  symTable_entry findSymbol(string symbolName);
  void updateOffset(string symbolName, int offset);
  void updateSection(string symbolName, string section);
  bool sectionExists(string section);
  void sortSections();
  void makeOutputMap();
  void makeOutputFile(string outFile);
  void setSectionStartAddr(int addr, string section);
  void relocateSections();
  void relocateSectionsWithPlace(list<section_places> *places);
  int getPoolSize(string section);
  void resolveRelocations();
  void resolveGlobalSymbols();
  vector<string> split(const string& str, char delimiter);
  string decimalToHex(int decimalNumber);
  int hexadecimalToDecimal(string hexVal);
  string formatOperandFourBytes(string value);
  string formatFourDigits(string value);
  string formatOperand(string value); //little endian
  int getLitNumInSection(string section);
  static bool sectionsSorter(const section_entry& lhs, const section_entry& rhs);
  void printGlobalSymTable();
  void printRelocTable();
  void printLiteralPoolMap(string outp);
  void printCodeMap(string outp);

  private:

    list<symTable_entry> *globalSymTable = new list<symTable_entry>();
    map<string, vector<char>> codeMap; //id sekcije, kod
    map<string, vector<char>> finalCodeMap;

    list <relocationTable_entry> *relTable=new list<relocationTable_entry>();
    list <section_entry> *sections = new list<section_entry>();  
    vector <literal_entry> literalPool = vector <literal_entry>();
    map<string,vector<string>> literalPoolMap;
};
  

#endif

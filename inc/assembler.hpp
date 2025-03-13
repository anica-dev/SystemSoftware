#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include "../inc/stuctures.hpp"
#include <iostream>
#include <vector>
#include <map>

using namespace std;


class Assembler
{

public:

  Assembler() {}

  void insertSymbol(string symbolName, string section);
  void insertSymbols(list<string> *symbols, string section);
  void insertGlobalSymbol(string symbolName, string section);
  void insertGlobalSymbols(list<string> *symbols, string section);
  void addNewSection(string section);
  int insertLiteralInPool(int literal, string section);
  int insertSymbolInPool(string symbol, string section);
  void setAddressInPool(int address, int literal, string section);
  void setSymbolAddressInPool(int address, string symbol, string section);
  //void setInPoolFlag(string symbol);
  string getOffset(int literal, string section);
  string getOffset(string symbol, string section);
  int getSymbolValue(string symbol);
  int getLitNumInSection(string section);
  bool symbolExists(string symbolName);
  void setGlobal(string symbolName);
  void setSectionSize(string section, int size);
  void setLiteralPoolAddr(int addr, string section);
  void setOffset(string symbolName, int offset, string section); //kada pronadje definiciju simbola da podesi offset
  void setSectionInSymTable(string symbolName, string section); //kada pronadje definiciju simbola da podesi sekciju
  string decimalToHex(int decimalNumber);
  string formatOperand(string value);
  string formatOperandFourBytes(string value);
  string formatLEFourBytes(int value);
  void addToRelocTable(int offset, string section, string symTableReference);
  void addToCode(string section, string byte);
  void addToCodeWordSymbols(string section, list<string> *symbols);
  void addToCodeWordLiterals(string section, list<int> *literals);
  bool sectionExists(string section);
  void addSectionToCode(string section);
  void makeOutputFile(string outFile);
  void makeSmallerOutputFile(string outFile);
  void errorLiteral(); //za instrukcije kod kojih ne sme da bude literal veci od 12b
  

  private:

  list<code_entry> *code = new list<code_entry>();
  map<string, vector<char>> mmap;
  list<symTable_entry> *symTable = new list<symTable_entry>();
  list <relocationTable_entry> *relTable = new list<relocationTable_entry>();
  vector <literal_entry> literalPool = vector <literal_entry>();
  vector <symPool_entry> symbolPool = vector <symPool_entry>();
  list<section_entry> *sections = new list <section_entry>();
};

#endif
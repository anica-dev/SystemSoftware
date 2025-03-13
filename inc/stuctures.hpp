#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <list>
#include <string>
#include <vector>

using namespace std;

typedef struct
  {
    int address;
    int type;
  } Info;

  typedef struct symTable_entry
  {
    string symbolName;
    string section;
    unsigned int symbolId;
    int offset; //adresa na kojoj je definicija simbola
    int isGlobal;
    int size; // velicina sekcije, -1 za simbole
    int fileId;

    symTable_entry(string symbolName, string section, int symbolId, int offset, int isGlobal, int size, int fileId){
      this->symbolName=symbolName;
      this->section=section;
      this->symbolId=symbolId;
      this->offset=offset;
      this->isGlobal=isGlobal;
      this->size=size;
      this->fileId=fileId;
    }

    symTable_entry(string symbolName){
      this->symbolName=symbolName;
    }
  } symTable_entry;

  typedef struct relocationTable_entry
  {
    int offset;  //mesto na koje treba da se umetne prava vrednost
    string section;
    string symTableReference;
    int symbolId;
    int fileId;

    relocationTable_entry(int offset, string section, string symTableReference, int symbolId, int fileId){
      this->offset=offset;
      this->section=section;
      this->symTableReference=symTableReference;
      this->symbolId=symbolId;
      this->fileId=fileId;
    }

  } relocationTable_entry;

  typedef struct code_entry{
      string section;
      vector<char> bytes;

      code_entry(string section){
        this->section=section;
      }
  } code_entry;

  typedef struct literal_entry
  {
    int literal;
    int address;
    string section;
    string fourBiteLiteral;
    
    literal_entry(int literal, int address, string section){
      this->literal=literal;
      this->address=address;
      this->section=section;
      this->fourBiteLiteral="";
    }
    literal_entry(string section){
      this->section=section;
      
    }
  } literal_entry;

  typedef struct symPool_entry
  {
    string symbol;
    int address;
    string section;
    int value;
    
    symPool_entry(string symbol, int address, string section, int value){
      this->symbol=symbol;
      this->address=address;
      this->section=section;
      this->value=value;
    }
  } symPool_entry;

  typedef struct section_entry
  {
    string sectionName;
    int fileId;
    int size;
    int poolAddr;
    int poolSize;
    unsigned int startingAddr;
    int relocated;
    
    section_entry(string sectionName){
      this->sectionName=sectionName;
      this->size=256;
      this->poolAddr=256;
      this->poolSize=-1;
      this->relocated=0;
    }
    section_entry(string sectionName, int id){
      this->sectionName=sectionName;
      this->fileId=id;
      this->size=256;
      this->poolAddr=256;
      this->poolSize=-1;
      this->relocated=0;
    }

    bool operator<(const section_entry& a){
      return startingAddr < a.startingAddr;
    }
  } section_entry;

  typedef struct section_places
  {
    string section;
    int address;
    int size;
    
    section_places(string section, int address){
      this->section=section;
      this->address=address;
      this->size=256;
    }
  } section_places;


#endif

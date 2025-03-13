#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "../inc/lexer.hpp"
#include "../inc/parser.hpp"
#include "../inc/assembler.hpp"
#include "../inc/linker.hpp"

using namespace std;


int main(int argc, char* argv[]) {

    std::vector<string> args;
    args.assign(argv, argv + argc);
    
    vector<string> p= vector<string>();
    vector<string> inputFiles = vector<string>();

    
    
    int i=2; //preskacemo -hex
    while(args[i]!="-o"){ 
      p.push_back(args[i]);
      i++;
      
    }
    i++; //za -o
    string outputFileName=args[i];
    i++;
    while(i<args.size()){
      inputFiles.push_back(args[i]);
      cout<<"input file: "<<args[i]<<endl;
      i++;
    }
    
  

    Linker* linker = new Linker();

    list<section_places> *places = new list<section_places>();
    for(int i=0;i<p.size();i++){
     
      vector<string> str = linker->split(p[i], '='); //-place=sekcija@adresa
      vector<string> s=linker->split(str[1], '@');
     
      vector<string> s2=linker->split(s[1], 'x');
      if(s[0]=="math") {
        
         section_places sp=section_places(s[0],stoul(s2[1],0,16)); //sekcija, adresa
         places->push_back(sp);
      }
      else if(s[0]=="my_code") {
       
        section_places sp=section_places(s[0],stoul(s2[1],0,16));
         places->push_back(sp);
      }
      else {
        section_places sp=section_places(s[0],stoul(s2[1],0,16));
         places->push_back(sp);
      }

      //cout<<"adresa "<<s2[1]<<" sekcija "<<s[0]<<endl;
      //linker->setSectionStartAddr(s2[1], s[0]); //s2[1] je adresa, s[0] sekcija
    }
    for(const auto &file: inputFiles){
    FILE *myfile1 = fopen(file.c_str(), "r");
      
	  if (!myfile1) {
    	cout << "I can't open file!" << endl;
    	return -1;
  	  }
    
    else linker->readFile(file);
    }
    linker->printGlobalSymTable();
    linker->printRelocTable();

    //linker->printCodeMap("codeMap.txt");
    //linker->printLiteralPoolMap("literalPoolMap.txt");
    //linker->resolveRelocations();
    //linker->printLiteralPoolMap("literalPoolMapAfter.txt");

    if(places->empty()) linker->relocateSections();
    else linker->relocateSectionsWithPlace(places);

    linker->makeOutputMap();
    linker->resolveGlobalSymbols();
    
    linker->makeOutputFile(outputFileName);

    return 0;
}

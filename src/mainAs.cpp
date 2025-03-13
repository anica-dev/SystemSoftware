#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "../inc/lexer.hpp"
#include "../inc/parser.hpp"
#include "../inc/assembler.hpp"

using namespace std;

extern Assembler* as;


int main(int argc, char* argv[]) {

    std::vector<string> args;
    args.assign(argv, argv + argc);
    //args[1] je -o
    string outputFileName=args[2];
    string inputFileName=args[3];
       
    args.assign(argv + 1, argv + argc);
    
    yydebug =0;
    
	FILE *myfile1 = fopen(inputFileName.c_str(), "r");
	 if (!myfile1) {
    		cout << "I can't open file!" << endl;
    		return -1;
  	}
	yyin = myfile1;

    // Call yyparse() to start parsing
    int result = yyparse();

    if (result == 0) {
        // Parsing succeeded
        std::cout << "Parsing succeeded!\n";
    } else {
        // Parsing failed
        std::cerr << "Parsing failed!\n";
    }
    as->makeSmallerOutputFile(outputFileName);
    as->makeOutputFile(inputFileName+"AssemOutput.txt");
   
    return 0;
}

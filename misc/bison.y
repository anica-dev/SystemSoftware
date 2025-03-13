/*** Definition section ***/
%define parse.trace

%{
	/* C code to be copied verbatim */
	 #include <iostream>
	 #include "../inc/assembler.hpp"
	 
	 
 	extern int yylex();
 	extern int yyparse();
 	
	void yyerror(const char *s) {
	fprintf(stderr, "Parser error: %s\n", s);
  }
	
	Assembler* as = new Assembler();

	string currentSection="undefined";

	int locationCounter=0;

	list<string> *symbol_list=new list<string>();
	list<int> *literal_list=new list<int>();
	int whatShift;

	string sp="e"; 
	string pc="f"; 	//hex
	string status="0";
	string handler="1";
	string cause="2";  //hex

	 
	using namespace std;
%}

%output "../src/parser.cpp"
%defines "../inc/parser.hpp"

%union {
  int num;
	long lNum;
	char* sym;
}

%token GLOBAL
%token EXTERN
%token SECTION
%token WORD
%token SKIP
%token END
%token HALT
%token INT
%token IRET
%token CALL
%token RET
%token JMP
%token BEQ
%token BNE
%token BGT
%token PUSH
%token POP
%token XCHG
%token ADD
%token SUB
%token MUL
%token DIV
%token NOT
%token AND
%token OR
%token XOR
%token SHL
%token SHR
%token LD
%token ST
%token CSRRD
%token CSRWR
%token <num>REG
%token <sym>CSR
%token COMMA
%token DOLLAR
%token <num> LITERAL
%token <sym> SYMBOL
%token LSQBRACE
%token RSQBRACE
%token PLUS
%token COLON


%%
/*** Rules section ***/

program : line
	| program line
  ;

line : code
			|
			label code
			;	

label: SYMBOL COLON {
				as->insertSymbol($1, currentSection);
				as->setOffset($1, locationCounter, currentSection);
				as->setSectionInSymTable($1, currentSection);
			}
      ;

code : directive
        | 
				instruction
        ;

directive : SECTION SYMBOL {
				if(currentSection!="undefined"){
					string disp = as->decimalToHex(256-locationCounter-4 + as->getLitNumInSection(currentSection)*4);
					int added=256-4 + as->getLitNumInSection(currentSection)*4;
				}
					if(currentSection!=""){
						as->setLiteralPoolAddr(256, currentSection);
					}
          as->insertSymbol($2, $2);
					as->addNewSection($2);
					currentSection=$2;
					as->setSectionSize(currentSection, 256);					
					locationCounter=0;
        }
        | EXTERN symbolList  {
				
         as->insertGlobalSymbols(symbol_list, currentSection);
        }
        | GLOBAL symbolList {
         as->insertGlobalSymbols(symbol_list, currentSection);
				 
        }
				| WORD symbolList  {
					as->addToCodeWordSymbols(currentSection, symbol_list);
					as->insertSymbols(symbol_list, currentSection);	
					locationCounter+=4;
				}
				| WORD literalList {
					as->addToCodeWordLiterals(currentSection, literal_list);
					locationCounter+=4;
				}
				| SKIP LITERAL {
					locationCounter+=$2;
					for (int i = 0; i < $2; i++)
						as->addToCode(currentSection, "00");
				}
				| END {
					if(currentSection!="")
						as->setSectionSize(currentSection, 256);
					string disp = as->decimalToHex(256-locationCounter-4 + as->getLitNumInSection(currentSection)*4);
					int added=256-4 + as->getLitNumInSection(currentSection)*4;
				
					locationCounter=0;
				}
        ;

symbolList : SYMBOL {
						string symbol=$1;
						symbol_list->push_back(symbol);
						as->addToRelocTable(locationCounter, currentSection, symbol);
						}
						|
						symbolList COMMA SYMBOL {
							string symbol=$3;
							symbol_list->push_back(symbol);
							as->addToRelocTable(locationCounter, currentSection, symbol);
						}
						;

literalList : LITERAL {
							int l=$1;
							literal_list->push_back(l);
						}
						|
						literalList COMMA LITERAL {
							int l=$3;
							literal_list->push_back(l);
						}
						;

instruction : HALT {
		
		as->addToCode(currentSection, "00000000");
		locationCounter+=4;
	}
	|
	INT {
		
		as->addToCode(currentSection, "10000000");
		locationCounter+=4;
	}
	|
	IRET {
		as->addToCode(currentSection, "91"+ sp + sp + "0800");   //sp<=sp+8 
		locationCounter+=4;
		as->addToCode(currentSection, "96" + status + sp + "0cff");  //status<=mem[sp-4]
		locationCounter+=4;
		as->addToCode(currentSection, "92"+ pc + sp + "08ff");  //pc<=mem[sp-8]
		locationCounter+=4;
	}
	|
	CALL LITERAL { //push pc; pc<=literal (push pc; pc<=mem[pc+disp])
		
		int literal=$2;
		as->insertLiteralInPool(literal, currentSection);
		string disp = as->getOffset(literal, currentSection);
		as->addToCode(currentSection, "21"+ pc + "00" + as->formatOperand(disp));
		locationCounter+=4;
	}
	|
	CALL SYMBOL {
		
		string sym=$2;
		as->insertSymbol(sym, currentSection);
		//as->addToRelocTable(locationCounter, currentSection, sym);
		//as->insertSymbolInPool(sym, currentSection);
		string disp = as->getOffset(sym, currentSection);
		as->addToCode(currentSection, "21"+ pc + "00" + as->formatOperand(disp));
		locationCounter+=4;
	}
	|
	RET {
		
		as->addToCode(currentSection, "93" + pc + sp+ "0400"); // pc<=mem[sp]; sp<=sp+4;
		locationCounter+=4;
  }
	| 
	JMP LITERAL {
		
		int literal = $2;
		as->insertLiteralInPool(literal, currentSection);
		string disp = as->getOffset(literal, currentSection);
		as->addToCode(currentSection, "38" + pc + "00"+ as->formatOperand(disp)); 
		locationCounter+=4;
	}
	|
	JMP SYMBOL {   //pc<=mem[pc+disp]
		
		string sym=$2;
		as->insertSymbol(sym, currentSection);
		//as->addToRelocTable(locationCounter, currentSection, sym);
		//as->insertSymbolInPool(sym, currentSection);
		string disp = as->getOffset(sym, currentSection);
		as->addToCode(currentSection, "38" + pc + "00"+ as->formatOperand(disp)); 
		locationCounter+=4;
	}
	|
	BEQ REG COMMA REG COMMA LITERAL { //if(regb==regc) pc<=mem[regA+disp]
		
		int literal = $6;
		string disp = as->getOffset(literal, currentSection);
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "39" + pc + reg1 + reg2 + as->formatOperand(disp));
		locationCounter+=4;
	}
	|
	BEQ REG COMMA REG COMMA SYMBOL { //if(regb==regc) pc<=mem[regA+disp]
		
		string sym=$6;
		as->insertSymbol(sym, currentSection);
		string disp = as->getOffset(sym, currentSection);
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "39" + pc + reg1 + reg2 + as->formatOperand(disp)); 
		locationCounter+=4;
	}
	|
	BNE REG COMMA REG COMMA LITERAL {
		
		int literal = $6;
		//as->insertLiteralInPool(literal, currentSection);
		string disp = as->getOffset(literal, currentSection);
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "3a" + pc + reg1 + reg2 + as->formatOperand(disp)); 
		locationCounter+=4;
	}
	|
	BNE REG COMMA REG COMMA SYMBOL {
		
		string sym=$6;
		as->insertSymbol(sym, currentSection);
		//as->addToRelocTable(locationCounter, currentSection, sym);
		//as->insertSymbolInPool(sym, currentSection);
		string disp = as->getOffset(sym, currentSection);
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "3a" + pc + reg1 + reg2 + as->formatOperand(disp)); 
		locationCounter+=4;
	}
	|
	BGT REG COMMA REG COMMA LITERAL {
		
		int literal = $6;
		//as->insertLiteralInPool(literal, currentSection);
		string disp = as->getOffset(literal, currentSection);
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "3b" + pc + reg1 + reg2 + as->formatOperand(disp)); 
		locationCounter+=4;
	}
	|
	BGT REG COMMA REG COMMA SYMBOL {
		
		string sym=$6;
		as->insertSymbol(sym, currentSection);
		//as->addToRelocTable(locationCounter, currentSection, sym);
		//as->insertSymbolInPool(sym, currentSection);
		string disp = as->getOffset(sym, currentSection);
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "3b" + pc + reg1 + reg2 + as->formatOperand(disp)); 
		locationCounter+=4;
	}
	|
	PUSH REG {
		
		string reg=as->decimalToHex($2);
		as->addToCode(currentSection, "81" + sp + "0" + reg + "cff"); // sp<=sp-4; mem[sp]<=reg;
		locationCounter+=4;
	}
	|
	POP REG {
		
		string reg=as->decimalToHex($2);
		as->addToCode(currentSection, "93" + reg + sp + "0400");  //reg<=mem[sp]; sp=sp+4
		locationCounter+=4;
	}
	|
	XCHG REG COMMA REG {
		
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "400" + reg1 + reg2 + "000");
		locationCounter+=4;
	}
	| 
	ADD REG COMMA REG {
		
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "50" + reg2 + reg2 + reg1 + "000");
		locationCounter+=4;
	}
	| 
	SUB REG COMMA REG {
		
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "51" + reg2 + reg2 + reg1 + "000");
		locationCounter+=4;
	}
	|
	MUL REG COMMA REG {
		
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "52" + reg2 + reg2 + reg1 + "000");
		locationCounter+=4;
	}
	|
	DIV REG COMMA REG {
		
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "53" + reg2+ reg2 + reg1 + "000");
		locationCounter+=4;
	}
	|
	NOT REG {
		
		string reg=as->decimalToHex($2);
		as->addToCode(currentSection, "60" + reg + reg + "0000");
		locationCounter+=4;
	}
	|
	AND REG COMMA REG {
		
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "61" + reg2 + reg2 + reg1 + "000");
		locationCounter+=4;
	}
	|
	OR REG COMMA REG {
		
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "62"+ reg2 + reg2 + reg1 + "000");
		locationCounter+=4;
	}
	|
	XOR REG COMMA REG {
		
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "63"+ reg2 + reg2 + reg1 + "000");
		locationCounter+=4;
	}
	|
	SHL REG COMMA REG {
	
		
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "70"+ reg2 + reg2 + reg1 + "000");
		locationCounter+=4;
	}
	|
	SHR REG COMMA REG {
		
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		as->addToCode(currentSection, "71"+ reg2 + reg2 + reg1 + "000");
		locationCounter+=4;
	}
	|
	load {
		
	}
	|
	store {
		
	}
	|
	CSRRD CSR COMMA REG {  //gpr<=csr
		
		string reg=as->decimalToHex($4);
		string csrVal=$2;
		int csr=0;   //status reg je 0
		if(csrVal=="%handler") csr=1;
	  else if(csrVal=="%cause") csr=2;

		as->addToCode(currentSection, "90"+ reg + to_string(csr) + "0000"); 
		locationCounter+=4;
	}
	|
	CSRWR REG COMMA CSR { //csr<=gpr
		
		string csrVal=$4;
		string reg=as->decimalToHex($2);
		int csr=0;   //status reg je 0
		if(csrVal=="%handler") csr=1;
		else if(csrVal=="%cause") csr=2;

		as->addToCode(currentSection, "94" + to_string(csr) + reg + "0000");
		locationCounter+=4;
	}
	;

	load : LD DOLLAR LITERAL COMMA REG { //reg<=literal (reg<= mem[pc+disp])
		string reg=as->decimalToHex($5);
		int literal=$3;
		as->insertLiteralInPool(literal, currentSection);
		string disp = as->getOffset(literal, currentSection);
		
		as->addToCode(currentSection, "92" + reg + pc + "0"+ as->formatOperand(disp));
		locationCounter+=4;
	}
	|	
	LD DOLLAR SYMBOL COMMA REG { // reg <= symbol (reg<= mem[pc+disp])
		string reg=as->decimalToHex($5);
		string sym=$3;
		as->insertSymbol(sym, currentSection);
		//as->addToRelocTable(locationCounter, currentSection, sym); //relokacioni zapis
		//as->insertSymbolInPool(sym, currentSection);
		string disp = as->getOffset(sym, currentSection);
		as->addToCode(currentSection, "92" + reg + pc + "0"+ as->formatOperand(disp)); 	
		locationCounter+=4;
	}
	|
	LD LITERAL COMMA REG { // reg<=mem[mem[pc+disp]] (memdir)
		string reg=as->decimalToHex($4);
		int literal=$2;
		as->insertLiteralInPool(literal, currentSection);
		string disp = as->getOffset(literal, currentSection);
		as->addToCode(currentSection, "92" + reg + pc + "0"+ as->formatOperand(disp)); // reg<=mem[pc+disp]
		locationCounter+=4;
		as->addToCode(currentSection, "92" + reg + reg + "0000"); // reg<= mem[reg]
		locationCounter+=4;
	}
	|
	LD SYMBOL COMMA REG { // reg<=mem[mem[pc+disp]]
		string reg=as->decimalToHex($4);
		string sym=$2;
		as->insertSymbol(sym, currentSection);
		//as->addToRelocTable(locationCounter, currentSection, sym);
		//as->insertSymbolInPool(sym, currentSection);
		string disp = as->getOffset(sym, currentSection);
		as->addToCode(currentSection, "92" + reg + pc + "0"+ as->formatOperand(disp)); // reg<=mem[pc+disp]
		locationCounter+=4;
		as->addToCode(currentSection, "92" + reg + reg + "0000"); // reg<= mem[reg]
		locationCounter+=4;
	}
	|
	LD REG COMMA REG {    //reg dir - u reg se upisuje vrednost iz drugog reg reg[a]<=reg[b]
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);

		as->addToCode(currentSection, "91" + reg1 + reg2 + "0000");
		locationCounter+=4;
	}
	|
	LD LSQBRACE REG RSQBRACE COMMA REG {  //reg ind - u reg se upisuje vrednost iz memorije na adresi reg
		string reg1=as->decimalToHex($3);   //reg[a]<=mem[reg[b]]
		string reg2=as->decimalToHex($6);

		as->addToCode(currentSection, "92" + reg1 + reg2 + "0000");
		locationCounter+=4;
	}
	|
	LD LSQBRACE REG PLUS LITERAL RSQBRACE COMMA REG { // ld [reg+literal], reg; reg ind sa pomerajem *izuzetak* nema bazena 
																										// reg2<=mem[reg1+literal]
		string reg1=as->decimalToHex($3);								
		string reg2=as->decimalToHex($8);
		string literal=as->decimalToHex($5);
		//as->insertLiteralInPool(literal, currentSection);
		//string disp = as->getOffset(literal, currentSection);
		
		if(literal.length()>12) as->errorLiteral();
		else as->addToCode(currentSection, "92" + reg2 + reg1 + "0"+ as->formatOperand(literal));
		locationCounter+=4;
	}
	|
	LD LSQBRACE REG PLUS SYMBOL RSQBRACE COMMA REG { // ld [reg+symbol], reg; reg ind sa pomerajem *izuzetak* nema bazena 
																										// reg2<=mem[reg1+symbol]
		string reg1=as->decimalToHex($3);
		string reg2=as->decimalToHex($8);
		string sym=$5;
		int symVal=as->getSymbolValue(sym);
		string symValHex=as->decimalToHex(symVal);
		as->insertSymbol(sym, currentSection);
		if(symVal==-1) as->errorLiteral();
		else if(symValHex.length()>12) as->errorLiteral();
		else as->addToCode(currentSection, "92" + reg2 + reg1 + "0"+ as->formatOperand(symValHex));
		locationCounter+=4;
	}
	;

	store : ST REG COMMA DOLLAR LITERAL { //literal<=reg tj mem[pc+disp]<=reg (immed, literal je u bazenu)
	  string reg=as->decimalToHex($2);
		int literal=$5;
		//as->insertLiteralInPool(literal, currentSection);
		string disp = as->getOffset(literal, currentSection);
		as->addToCode(currentSection, "80" + pc + "0" + reg + as->formatOperand(disp));
		locationCounter+=4;
	}
	|	
	ST REG COMMA DOLLAR SYMBOL {  //symbol<=reg
		string reg=as->decimalToHex($2);
		string sym=$5;
		as->insertSymbol(sym, currentSection);
		//as->addToRelocTable(locationCounter, currentSection, sym);
		//as->insertSymbolInPool(sym, currentSection);	
		string disp = as->getOffset(sym, currentSection);
		as->addToCode(currentSection, "80" + pc + "0" + reg + as->formatOperand(disp));
		locationCounter+=4;
	}
	|
	ST REG COMMA LITERAL { //mem[literal]<=reg tj mem[mem[pc+disp]]<=reg (mem dir)
		string reg=as->decimalToHex($2);
		int literal=$4;
		//as->insertLiteralInPool(literal, currentSection);
		string disp = as->getOffset(literal, currentSection);
		as->addToCode(currentSection, "82" + pc +  "0" + reg + as->formatOperand(disp));
		locationCounter+=4;
	}
	|
	ST REG COMMA SYMBOL {  //mem[symbol]<=reg (mem dir)
		string reg=as->decimalToHex($2);
		string sym=$4;
		as->insertSymbol(sym, currentSection);
		//as->addToRelocTable(locationCounter, currentSection, sym);
		//as->insertSymbolInPool(sym, currentSection);
		string disp = as->getOffset(sym, currentSection);
		as->addToCode(currentSection, "82" + pc +  "0" + reg + as->formatOperand(disp));
		locationCounter+=4;
	}
	|
	ST REG COMMA REG {  //reg direktno
			string reg1=as->decimalToHex($2);
			string reg2=as->decimalToHex($4);

			as->addToCode(currentSection, "91"+ reg2 + reg1 + "0000"); 
			locationCounter+=4;
	}
	|
	ST REG COMMA LSQBRACE REG RSQBRACE { //reg indirektno
			string reg1=as->decimalToHex($2);
			string reg2=as->decimalToHex($5);

			as->addToCode(currentSection, "80"+ reg2 +"0"+ reg1 + "000"); //mem[gpr[A]+gpr[B]+D]<=gpr[C];
																																		//mem[reg2]<=reg1;
			locationCounter+=4;
	}
	|
	ST REG COMMA LSQBRACE REG PLUS LITERAL RSQBRACE { // st reg, [reg+literal] reg ind sa pomerajem *izuzetak* nema bazena
																										//mem[reg2+literal]<=reg1
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($5);
		string literal=as->decimalToHex($7);
		//as->insertLiteralInPool(literal, currentSection);
		//string disp = as->getOffset(literal, currentSection);
		if(literal.length()>12) as->errorLiteral();
		else as->addToCode(currentSection, "80" + reg2 +  "0" + reg1 + as->formatOperand(literal));
		locationCounter+=4;
	}
	|
	ST REG COMMA LSQBRACE REG PLUS SYMBOL RSQBRACE { // st reg, [reg+symbol] reg ind sa pomerajem *izuzetak* nema bazena
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($5);
		string sym=$7;
		int symVal=as->getSymbolValue(sym);
		string symValHex=as->decimalToHex(symVal);
		as->insertSymbol(sym, currentSection);
		if(symVal==-1) as->errorLiteral();
		else if(symValHex.length()>12) as->errorLiteral();
		else as->addToCode(currentSection, "80" + reg2 +  "0" + reg1 + as->formatOperand(symValHex));
		locationCounter+=4;
	}
	;




%%
/*** C Code section ***/


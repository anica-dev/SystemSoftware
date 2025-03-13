/*** Definition section ***/
%{
	#include "../inc/parser.hpp"
  	#include <iostream> 
  	
  	extern int yylex();

  	using namespace std;
%}

%option outfile="../src/lexer.cpp" header-file="../inc/lexer.hpp"
%option noyywrap

%%

 /*** Rules section ***/


\.global { return GLOBAL; }
\.extern { return EXTERN; }
\.section { return SECTION; }
\.word { return WORD; }
\.skip { return SKIP; }
\.end { return END; }
halt { return HALT; }
int { return INT; }
iret { return IRET; }
call { return CALL; }
ret { return RET; }
jmp { return JMP; }
beq { return BEQ; }
bne { return BNE; }
bgt { return BGT; }
push { return PUSH; }
pop { return POP; }
xchg { return XCHG; }
add { return ADD; }
sub { return SUB; } 
mul { return MUL; }
div { return DIV; }
not { return NOT; }
and { return AND; }
or { return OR; }
xor { return XOR; }
shl { return SHL; }
shr { return SHR; }
ld { return LD; }
st { return ST; }
csrrd { return CSRRD; }
csrwr { return CSRWR; }
"," { return COMMA; }
"$" { return DOLLAR; }
"[" { return LSQBRACE; }
"]" { return RSQBRACE; }
"+" { return PLUS; }
":" { return COLON; }

%r[0-9] { yylval.num = atoi(yytext+2); return REG; }
%r1[0-5] { yylval.num = atoi(yytext+2); return REG; }
%sp { yylval.num=14; return REG; }
%pc { yylval.num=15; return REG; }
%status { yylval.sym = strdup(yytext); return CSR; }
%handler { yylval.sym = strdup(yytext); return CSR; }
%cause {  yylval.sym = strdup(yytext); return CSR; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.sym = strdup(yytext); return SYMBOL; }
[0-9]+ { yylval.num = atoi(yytext); return LITERAL; } //decimalni brojevi
0[xX][0-9a-fA-F]+ { yylval.lNum = stol(yytext+2, nullptr, 16);  return LITERAL; } //hex

#.*\n { }     //komentar     
[ \t\n]+ ;

. ;

%%

/*** C Code section ***/

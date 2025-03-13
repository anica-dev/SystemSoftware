#include "../inc/emulator.hpp"
#include "../inc/linker.hpp"
#include <vector>
#include <string>
#include <list>

using namespace std;

vector<int> poolSizes;

void Emulator::readFile(string filename){
  ifstream inputFile(filename);

    if (!inputFile.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    string line;
    int digitsNum=0;
    unsigned int addrStart=stoi("40000000", 0, 16);
    while(1){
      getline(inputFile, line);
      if(line=="END") break;  
      vector<string> s = split(line, ':');
      vector<string> b = split(s[1], ' ');
      string bytes;
      string startingAdr=s[0];
      unsigned int adr=uint(stoul(startingAdr,0,16));
      for(int i=0;i<b.size();i++){
        for(char ch : b[i]){
          bytes.push_back(ch);
          digitsNum++;
          if(digitsNum==8){
            memory.insert({adr, bytes});
            digitsNum=0;
            bytes.clear();
            adr+=4;
          }
        }
      }   
    }
   
}

vector<string> Emulator::split(const string &str, char delimiter)
{
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}


void Emulator::emulate(){
  regs[pc] = stoi("40000000", 0, 16);
  
  int num=0;
  int sectionNum=0;
  string instr=memory[regs[pc]];
  while(instr!=""){
    num+=4;
    instr= memory[regs[pc]];
    //cout<<"reg[sp] "<<regs[14]<<endl;
    if(instr=="") break;
    //cout<<"instr: "<<memory[regs[pc]]<<endl;
    regs[pc] += 4;
    int op=stoi(instr.substr(0, 1), 0, 16); //16 oznacava hex
    int mod = stoi(instr.substr(1, 1), 0, 16);
    int a = stoi(instr.substr(2, 1), 0, 16); //index 3, duzina 1 
    int b = stoi(instr.substr(3, 1), 0, 16);
    int c = stoi(instr.substr(4, 1), 0, 16);
    //cout<<"op "<<op<<endl;
    //cout<<"a b c "<<a<<" "<<b<<" "<<c<<endl;
    string lowerBytes = instr.substr(6,2);
    string higherByte = instr.substr(5,1);
    int disp;
    string dispString = lowerBytes+higherByte;
    //cout<<"dispString "<<dispString<<endl;
    if(dispString=="ffc"){
      disp=-4;
    } 
    else if(dispString=="ff8"){
      disp=-8;
    } 
    else {
      disp = stoi(dispString, 0, 16);
    }
    //cout<<"disp "<<lowerBytes+higherByte<<endl;
    //cout<<"disp "<<disp<<endl;
    switch (op){
    case 0:  
      halt();
      break;
    case 1: 
      interrupt();
      break;
    case 2:  
      switch (mod)
      {
      case 0: callMod0(a, b, disp); break;
      case 1: callMod1(a, b, disp); break;
      default: break;
      }
      break;
    case 3:  
      switch(mod){
        case 0: jumpDirect(a, disp); break;
        case 1: break;
        case 2: break;
        case 3: break;
        case 8: jump(a,disp); break;
        case 9: jumpEQ(a,b,c,disp);break;
        case 10: jumpNEQ(a,b,c,disp); break;
        case 11: jumpGT(a,b,c,disp); break;
        default:break;
      }
      break;
    case 4: 
      xchg(b, c);
      break;
    case 5:
      switch (mod)
      {
      case 0: add(a, b, c); break;
      case 1: sub(a, b, c); break;
      case 2: mul(a, b ,c); break;
      case 3: div(a, b ,c); break;
  /*  case 4: addShl(a,b, c, disp); break;
      case 5: addShr(a,b, c, disp); break;
      case 6: subShl(a,b, c, disp); break;
      case 7: subShr(a,b, c, disp); break;  */
      default: break;
      }
      break;
    case 6: 
      switch (mod)
      {
      case 0: notInstr(a, b); break;
      case 1: andInstr(a, b, c); break;
      case 2: orInstr(a, b ,c); break;
      case 3: xorInstr(a, b ,c); break;
      default: break;
      }
      break;
    case 7:
      switch (mod)
      {
      case 0: shiftLeft(a, b, c); break;
      case 1: shiftRight(a, b, c); break;
      default: break;
      }
      break;
    case 8: 
      switch (mod)
      {
      case 0: storeMod0(a,b,c,disp); break; //mem[reg[a]+reg[b]+d]<=reg[c]
      case 2: storeMod2(a,b,c,disp);break;  //mem[mem[reg[a]+reg[b]+d]]<=reg[c]
      case 1: push(regs[c]); break; //reg[c] se pushuje, a je sp, disp je 4
      default:break;
      }
      break;
    case 9: //load
      switch(mod){
        case 0: csrrd(a, b); break;
        case 1: loadInReg(a,b,disp); break;
        case 2: load(a,b,c,disp); break;
        case 3: pop(a,b); break; //prvo je reg u koji se upisuje, drugo je sp
        case 4: csrwr(a,b); break;
        case 5: break;
        case 6: loadCsr(a,b,c,disp); break;
        case 7: popCsr(a,b,disp); break;
        default:break;
      }
      break;
    default:
      cout << "Unknown instruction." << endl;
      endProg = true;
      break;
    }
  }  
}

void Emulator::printOutput(string outFile){
   ofstream output;
    output.open(outFile);
  output<<"Emulated processor executed halt instruction"<<endl<<"Emulated processor state:"<<endl;
  for(auto i=memory.begin();i!=memory.end();i++){
        output<< decimalToHex(i->first) << ": ";
        for(auto i2=0;i2!=i->second.size();i2++){
            output<< i->second[i2];
        }
        output<<"\n";
    }

    for(int i=0;i<16;i++){
    cout<<"r"<<i<<"=0x"<<formatOperandFourBytes(decimalToHex(regs[i]));
    if(i % 4 == 3){
      cout<<endl;
    } 
    else {
      cout<<"\t";
    }
  }
}

string Emulator::formatRegisterValue(unsigned int value){

  std::stringstream stream;
  stream << std::setfill('0') << std::setw(8) << std::hex << value;
  std::string hex_string = stream.str();

  return hex_string;
}

 string Emulator::formatOperandFourBytes(string value) {
        string val;
        if(value.length() == 1){
            val = "0000000" + value;
        }
        else if(value.length() == 2){
            val = "000000" + value;
        }
        else if(value.length() == 3){
            val = "00000" + value;
        }
        else if(value.length() == 4){
            val = "0000" + value;
        }
        else if(value.length() == 5){
            val = "000" + value;
        }
        else if(value.length() == 6){
            val = "00" + value;
        }
        else if(value.length() == 7){
            val = "0" + value;
        }
        else{
            val = value;
        }
        
        return val;
    } 
int Emulator::littleEndianHexToInt(string input) {
    string hexStr;
    if(input.size() == 8){
      string b0 = input.substr(0, 2);
      string b1 = input.substr(2, 2);
      string b2 = input.substr(4, 2);
      string b3 = input.substr(6, 2);
      hexStr = b3 + b2 + b1 + b0;
      //cout<<"hexStr "<<hexStr<<endl;
    }
    if(input.size() == 3){
      string b0=input.substr(0,1);
      string b1=input.substr(1,2);
      hexStr=b1+b0;
    }
    //int result = stoi(hexStr, 0, 16); //this stoi throws an error
    unsigned int result = uint(stoul(hexStr, 0, 16));
    return result;
}

string Emulator::formatLittleEndian(int val){
  std::stringstream stream;
  stream << std::setfill('0') << std::setw(2) << std::hex << (val & 0xFF)
           << std::setw(2) << (val >> 8 & 0xFF)
           << std::setw(2) << (val >> 16 & 0xFF)
           << std::setw(2) << (val >> 24 & 0xFF);
  return stream.str();
}

string Emulator::decimalToHex(int decimalNumber){
        std::stringstream ss;
        ss << std::hex << decimalNumber;
        string s=ss.str();
        formatOperandFourBytes(s);
        return s;
    }

void Emulator::halt(){
  endProg=true;
  cout<<"Halt"<<endl;
}

void Emulator::interrupt(){
  cout<<"Interrupt!"<<endl;
  push(csr[status]);
  push(regs[pc]);
  csr[cause] = 4;
  csr[status] = csr[status] & (~0x1);
  regs[pc] = csr[handler];
  //cout<<"pc: "<<regs[pc]<<endl;
}

void Emulator::callMod0(int a, int b, int disp){
  push(regs[pc]);
  regs[pc]=regs[a]+regs[b]+disp;
}

void Emulator::callMod1(int a, int b, int disp){
  //cout<<"call1"<<endl;
  push(regs[pc]);
  //cout<<"pc pushed on stack, pc value: "<<decimalToHex(regs[pc])<<endl;
  regs[pc]=uint(stoul(memory[regs[a]+regs[b]+disp],0,16));
}

void Emulator::jumpDirect(int a, int disp){
  regs[pc]=regs[a]+disp;
}

void Emulator::jump(int a, int disp){
  regs[pc]=stoi(memory[regs[a]+disp],0,16);
}

void Emulator::jumpEQ(int a, int b, int c, int disp){
  //cout<<regs[b]<< " EQUAL? "<<regs[c]<<endl;
  //cout<<"regs[a] "<<decimalToHex(regs[a])<<", disp: "<<disp<<endl;
  //cout<<"regs[a]+disp="<<decimalToHex(regs[a]+disp)<<endl;
  if(regs[b]==regs[c])
    regs[pc]=stoi(memory[regs[a]+disp],0,16);
    //cout<<"regs[a]+disp="<<regs[a]+disp<<endl;
    //cout<<"pc: "<<regs[pc]<<endl;
}

void Emulator::jumpNEQ(int a, int b, int c, int disp){
  if(regs[b]!=regs[c])
    regs[pc]=stoi(memory[regs[a]+disp],0,16);
}

void Emulator::jumpGT(int a, int b, int c, int disp){
  if(regs[b]>regs[c]) //reg b signed??
    regs[pc]=stoi(memory[regs[a]+disp],0,16);
}

void Emulator::xchg(int a, int b){
  int temp=regs[a];
  regs[a]=regs[b];
  regs[b]=temp; 
}

void Emulator::add(int a, int b, int c){
  cout<<"Add "<<regs[b]<<" to "<<regs[c]<<endl;
  regs[a]=regs[b]+regs[c];
}

void Emulator::sub(int a, int b, int c){
  cout<<"Sub "<<regs[b]<<" from "<<regs[c]<<endl;
  regs[a]=regs[b]-regs[c];
}

void Emulator::mul(int a, int b, int c){
  cout<<"Mul "<<regs[b]<<" with "<<regs[c]<<endl;
  regs[a]=regs[b]*regs[c];
}

void Emulator::div(int a, int b, int c){
  cout<<"Division "<<regs[b]<<" with "<<regs[c]<<endl;
  if(regs[c]==0) cout<<"Error: Division with zero!"<<endl;
  regs[a]=regs[b]/regs[c];
}

void Emulator::shiftLeft(int a, int b, int c){
  regs[a] = regs[b]<<regs[c];
}

void Emulator::shiftRight(int a, int b, int c){
  regs[a] = regs[b]>>regs[c];
}

void Emulator::push(int val){
  cout<<"Push "<<val<<endl;
  
  regs[sp]-=4;
  //cout<<"reg[sp] "<<decimalToHex(regs[sp])<<endl;
  string valStr=formatOperandFourBytes(decimalToHex(val));
  memory[regs[sp]]=valStr;
  //memory.insert({regs[sp], valStr});
}

void Emulator::csrrd(int a, int b){
  //cout<<"csrrd "<<csr[b]<<endl;
  regs[a]=csr[b];
  //cout<<"da li je csrrd ucitao ono sto treba? ucitao je: "<<regs[a]<<endl;
}

void Emulator::csrwr(int a, int b){
  //cout<<"csrwr"<<endl;
  csr[a]=regs[b];
  //cout<<"csr["<<a<<"]="<<decimalToHex(csr[a])<<endl;
}

void Emulator::loadInReg(int a,int b,int disp){
  regs[a]=regs[b]+disp;
}

void Emulator::load(int a, int b, int c, int disp){
/*  cout<<"disp "<<decimalToHex(disp)<<endl;
  cout<<"reg[a] "<<regs[a]<<endl;
  cout<<"reg[sp] "<<decimalToHex(regs[b])<<endl;
  cout<<"reg[c] "<<decimalToHex(regs[c])<<endl;*/
  //cout<<"adr sa koje se ucitava "<<decimalToHex(regs[c]+regs[b]+disp)<<endl;*/
 //cout<<"memory[regs[b]+regs[c]+disp] "<<memory[regs[b]+regs[c]+disp]<<endl;
  
  regs[a]=uint(stoul(memory[regs[b]+regs[c]+disp],0,16));
  
  //cout<<"loaded "<<decimalToHex(regs[a])<<endl;
}

void Emulator::loadCsr(int a, int b, int c, int disp){
}

int Emulator::pop(int a, int b){
  //cout<<"Pop"<<endl;
  //cout<<"argument "<<memory[regs[sp]]<<endl;
  //cout<<"reg[sp] "<<decimalToHex(regs[sp])<<endl;
  int val=uint(stoul(memory[regs[sp]],0,16));
  regs[a]=uint(stoul(memory[regs[b]],0,16));
  auto it = memory.find(regs[sp]);  
  if (it != memory.end()) { 
        memory.erase(it); 
  } 
  regs[b]+=4;
  cout<<"Pop val "<<val<<endl;
  return val;
}

int Emulator::popCsr(int a, int b, int disp)
{
  return 0;
}

void Emulator::storeMod0(int a, int b, int c, int disp){
  memory[regs[a]+regs[b]+disp]=formatOperandFourBytes(decimalToHex(regs[c]));
}

void Emulator::storeMod2(int a, int b, int c, int disp){
  unsigned int r=uint(stoul(memory[regs[a]+regs[b]+disp],0,16));
  memory[r]=decimalToHex(regs[c]);
}

void Emulator::notInstr(int a, int b){
  regs[a] = ~regs[b];
}

void Emulator::andInstr(int a, int b, int c){
  regs[a] = regs[b]&regs[c];
}

void Emulator::orInstr(int a, int b, int c){
  regs[a] = regs[b]|regs[c];
}

void Emulator::xorInstr(int a, int b, int c){
  regs[a] = regs[b]^regs[c];
}
/*
void Emulator::addShl(int a, int b, int c, int disp){ //reg2, reg2, reg1
  push(regs[c]); //reg1
  int shifted=regs[c] << disp;
  regs[a] = regs[a] + shifted;
  pop(regs[c], sp);
}

void Emulator::addShr(int a, int b, int c, int disp){
  push(regs[c]); //reg1
  int shifted=regs[c] >> disp;
  regs[a] = regs[a] + shifted;
  pop(regs[c], sp);
}

void Emulator::subShl(int a, int b, int c, int disp){
  push(regs[c]); //reg1
  int shifted=regs[c] << disp;
  regs[a] = regs[a] - shifted;
  pop(regs[c], sp);
}

void Emulator::subShr(int a, int b, int c, int disp){
  push(regs[c]); //reg1
  int shifted=regs[c] >> disp;
  regs[a] = regs[a] - shifted;
  pop(regs[c], sp);
}
*/

/*
|
	ADD REG COMMA REG COMMA SHL LITERAL{
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		int literal=$7;
		as->addToCode(currentSection, "54" + reg2 + reg2 + reg1 + as->formatOperand(to_string(literal)));
		locationCounter+=4;	
	}
	|
	ADD REG COMMA REG COMMA SHR LITERAL{
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		int literal=$7;
		as->addToCode(currentSection, "55" + reg2 + reg2 + reg1 + as->formatOperand(to_string(literal)));
		locationCounter+=4;	
	}
	|
	SUB REG COMMA REG COMMA SHL LITERAL{
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		int literal=$7;
		
		as->addToCode(currentSection, "56" + reg2 + reg2 + reg1 + as->formatOperand(to_string(literal)));
		locationCounter+=4;	
	}
	|
	SUB REG COMMA REG COMMA SHR LITERAL{
		string reg1=as->decimalToHex($2);
		string reg2=as->decimalToHex($4);
		int literal=$7;
		
		as->addToCode(currentSection, "57" + reg2 + reg2 + reg1 + as->formatOperand(to_string(literal)));
		locationCounter+=4;	
	}
*/

/* 
    add %r2, %r3, shl 2 #r3=r3+r2<<2 = 3+8=11
    add %r6, %r4, shr 1 #r4=r4+r6>>1 = 4+3=7
    sub %r2, %r5, shl 1 #r5=r5-r2<<1 = 5-4=1
    sub %r2, %r6, shr 1 #r6=r6-r2>>1 = 6-1=5
*/
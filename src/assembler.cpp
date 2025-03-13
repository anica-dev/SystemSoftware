#include "../inc/lexer.hpp"
#include "../inc/parser.hpp"
#include "../inc/assembler.hpp"
#include <fstream>
#include <list>
#include <vector>
#include <iomanip>

using namespace std;

std::ofstream outputFile;
int symbolCount=0;

    void Assembler::insertSymbol(string symbolName, string section){
        if(!symbolExists(symbolName)){
            symTable_entry s=symTable_entry(symbolName, section,symbolCount, -1 , 0, -1, 0);
            symTable->push_back(s);
            symbolCount++;
        }
    }

    void Assembler::insertSymbols(list<string> *symbols, string section){
        while(symbols->size()!=0){
            auto i=symbols->begin();
            string s=*i;
            insertSymbol(s, section);
            symbols->pop_front();
        }
    }
    void Assembler::insertGlobalSymbol(string symbolName, string section){
        //cout<<" global symbol: "<<symbolName<<", section: "<<section<<endl;
        if(!symbolExists(symbolName)){
            symTable_entry s=symTable_entry(symbolName, section,symbolCount, -1 , 1, -1, 0);
            symTable->push_back(s);
            symbolCount++;
        }
        else{
            setGlobal(symbolName);
        }
    }

    void Assembler::insertGlobalSymbols(list<string> *symbols, string section){
        while(symbols->size()!=0){
            auto i=symbols->begin();
            string s=*i;
            insertGlobalSymbol(s, section);
            symbols->pop_front();
        }
    }

    void Assembler::addNewSection(string section){
        section_entry s=section_entry(section);
        sections->push_back(s);
    }

    int Assembler::getSymbolValue(string symbol){
        int val=-1;
        for(auto i=symTable->begin();i!=symTable->end();i++){
            if(symbol==i->symbolName)
                val=i->offset;
                break;
        }
        return val;
    }

    int Assembler::getLitNumInSection(string section){
        int num=0;
        for(int i=0;i<literalPool.size();i++)
            if(literalPool[i].section==section) num++;

      /*  for(int i=0;i<symbolPool.size();i++)
            if(symbolPool[i].section==section) num++; */
        return num;
    }

    int Assembler::insertLiteralInPool(int literal, string section){
        literal_entry l = literal_entry(literal, -1, section);
        bool exists=false;
        for(int i=0;i<literalPool.size();i++){
            if(literal==literalPool[i].literal && section==literalPool[i].section){
                exists=true;           
                return literalPool[i].address; 
            }
        }
        if(!exists){
            literalPool.push_back(l);
            int poolAddr=256;
            int numOfLiterals=getLitNumInSection(section); // litNum predstavlja broj literala + broj simbola
            //cout<<"num of literals: "<<numOfLiterals<<endl;
            int location = poolAddr + (numOfLiterals - 1) * 4;
            setAddressInPool(location, literal, section);
            //cout<<"calculated location: "<<location<<endl;
            return location;
        }
        return -1;
    }

    int Assembler::insertSymbolInPool(string symbol, string section){
      /*  symPool_entry s=symPool_entry(symbol, -1, section,0);
        bool exists=false;
         for(int i=0;i<symbolPool.size();i++){
            if(symbol==symbolPool[i].symbol && section==literalPool[i].section){
                exists=true;           
                return symbolPool[i].address; 
            }
        }*/
        int symVal=getSymbolValue(symbol);
        literal_entry l=literal_entry(section);
        if(symVal!=-1) {
            l.literal=symVal;
        }
        else{
            l.literal=0;
        }
            literalPool.push_back(l);
            int poolAddr=256;   
            int numOfLiterals=getLitNumInSection(section);
            //cout<<"num of literals: "<<numOfLiterals<<endl;
            int location = poolAddr + (numOfLiterals - 1) * 4;
            addToRelocTable(location, section, symbol);
            //setSymbolAddressInPool(location, symbol, section); 
            //setAddressInPool(location, literal, section);
            //cout<<"calculated location symbol: "<<location<<endl;
            return location;
        
    }

    void Assembler::setAddressInPool(int address, int literal, string section){
        for(auto i=0;i<literalPool.size();i++){
            if(literalPool[i].literal==literal && literalPool[i].section==section){
               literalPool[i].address=address;  
            }
        }
    }

    void Assembler::setSymbolAddressInPool(int address, string symbol, string section){
        for(auto i=0;i<symbolPool.size();i++){
            if(symbolPool[i].symbol==symbol && literalPool[i].section==section){
               symbolPool[i].address=address;
            }
        }
    }

/*
    void Assembler::setInPoolFlag(string symbol){
        for(auto i=relTable->begin();i!=relTable->end();i++){
            if(symbol==i->symTableReference){
                i->inPool=1;
            }
        }
    }
*/

    extern int locationCounter;
    string Assembler::getOffset(int literal, string section){  
        int address=insertLiteralInPool(literal, section);     
        int offset= address - locationCounter - 4;
        cout<<"offset: "<<decimalToHex(offset)<<endl;
        return decimalToHex(offset);
    }


    string Assembler::getOffset(string symbol, string section){
        int offset;
       /* bool defined=false;
        for(auto i=symTable->begin();i!=symTable->end();i++){
            if(i->symbolName==symbol && i->offset!=-1){
                offset=i->offset;
                defined=true;
            }
        }
        if(defined){  
            int address=insertLiteralInPool(offset, section);
            int offset= address - locationCounter - 4;
        }
        else{
            int address=insertLiteralInPool(0, section);
            int offset= address - locationCounter - 4;
        }
        */
        int address=insertSymbolInPool(symbol, section);     
        offset= address - locationCounter - 4;
        cout<<"offset symbol: "<<offset<<endl;
        return decimalToHex(offset);
    }

    bool Assembler::symbolExists(string symbolName){
        int exists=false;
        for(auto i=symTable->begin();i!=symTable->end();i++){
            symTable_entry s=*i;
            if(symbolName==s.symbolName) {
                exists=true;
                break;
            }
        }
        return exists;
    }

    void Assembler::setGlobal(string symbolName){
        for(auto i=symTable->begin();i!=symTable->end();i++){
            if(symbolName==i->symbolName){
                i->isGlobal=1;
                break;
            }
        }
    }
    
    void Assembler::setSectionSize(string section, int size){
        for(auto i=symTable->begin();i!=symTable->end();i++){
            if(section==i->symbolName){
                i->size=size;
                break;
            }
        }
    }

    void Assembler::setLiteralPoolAddr(int addr, string section){
        for(auto i=sections->begin();i!=sections->end();i++){
            if(i->sectionName==section) i->poolAddr=addr;
        }
    }

    void Assembler::setOffset(string symbolName, int offset, string section){
        for(auto i=symTable->begin();i!=symTable->end();i++){
            if(symbolName==i->symbolName){
                i->offset=offset;
                i->section=section;
                //cout<<"offset is set for symbol "<<symbolName<<" and it is "<<i->offset<<" and section is "<<i->section<<endl;
                break;
            }
        }
    }

     void Assembler::setSectionInSymTable(string symbolName, string section){
      /* for(auto i=symTable->begin();i!=symTable->end();i++){
            if(symbolName==i->symbolName){
                i->section=section;
                
                break;
            }
        }
        */
    }

    string Assembler::decimalToHex(int decimalNumber){
        std::stringstream ss;
        ss<<std::hex << decimalNumber;
        string s=ss.str();
        return s;
    }

    string Assembler::formatOperand(string value) {
        string val;
        if(value.length() == 1){
            val = "00" + value;
        }
        else if(value.length() == 2){
            val = "0" + value;
        }
        else{
            val = value;
        }
        string higherBytes = val.substr(0, 2);
        string lowerByte = val.substr(2, 1);
        return lowerByte+higherBytes;
    } 
    string Assembler::formatOperandFourBytes(string value) {
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

    string Assembler::formatLEFourBytes(int value) {
        std::stringstream stream;
        
        stream << std::setfill('0') << std::setw(2) << std::hex << (value & 0xFF)
                << std::setw(2) << (value >> 8 & 0xFF)
                << std::setw(2) << (value >> 16 & 0xFF)
                << std::setw(2) << (value >> 24 & 0xFF);
        return stream.str();
    }  

    void Assembler::addToRelocTable(int offset, string section, string symTableReference){
        for(auto i=symTable->begin();i!=symTable->end();i++){
            if(i->symbolName==symTableReference){//&& i->offset==-1
                relocationTable_entry s=relocationTable_entry(offset, section, symTableReference, 0 ,0);
                relTable->push_back(s);
            }
        }
            
    }

    void Assembler::addToCode(string section, string byte){
        //cout<<"loc counter: "<<locationCounter<<endl;
    vector<char> vektor;
        if(!sectionExists(section)){
            addSectionToCode(section);
        }
        for(auto i=code->begin();i!=code->end();i++){
            code_entry s=*i;
            if(s.section==section){
                
                for (char ch : byte) {
                    s.bytes.push_back(ch);
                    vektor.push_back(ch);
                }
            }   
        }
        //map
        int i=0;
        for (char ch : byte) {
            mmap[section].push_back(ch);
            //cout<<"map ch: "<<mmap[section].at(i)<<" ";
            i++;
        }
           
    }

    void Assembler::addToCodeWordLiterals(string section, list<int> *literals){
        while(literals->size()!=0){
            auto i=literals->begin();
            int literal=*i;
            addToCode(section,formatLEFourBytes((literal)));
            literals->pop_front();
        } 
    }

    void Assembler::addToCodeWordSymbols(string section, list<string> *symbols){
        while(symbols->size()!=0){
            auto i=symbols->begin();
            string symbol=*i;
            addToCode(section,"00000000");
            symbols->pop_front();
        }
    }

    bool Assembler::sectionExists(string section){
        bool exists=false;
        for(auto i=code->begin();i!=code->end();i++){
            code_entry s=*i;
            if(s.section==section){
                exists=true;
                break;
            }
        }
        return exists;
    }

    void Assembler::addSectionToCode(string section){
        code_entry c=code_entry(section);
        code->push_back(c);
        //map
        vector<char> bytes=vector<char>();
        mmap.insert({section,bytes});
    }


    void Assembler::makeOutputFile(string outFile){
        ofstream output;
        output.open(outFile);

        /* SYMBOL TABLE */
        output<<"Symbol table\n"<<endl;
        output << std::left 
                << std::setw(20) << "Symbol" 
                <<std::setw(20) << "Section" 
                <<std::setw(20) << "Symbol ID" 
                << std::setw(20) << "Offset" 
                <<std::setw(20) << "Is global"
                <<std::setw(20) << "Size" << endl;

        for(auto i=symTable->begin();i!=symTable->end();i++){
            symTable_entry s=*i;           
           
            output << std::left << std::setw(20) << s.symbolName
            << std::setw(20) << s.section
            << std::setw(20) << s.symbolId
            << std::setw(20) << s.offset 
            << std::setw(20) << s.isGlobal 
            << std::setw(20) << s.size<<endl;
        }

        output<<"\n"<<endl;

        /* RELOCATION TABLE */
        output<<"Relocation table\n"<<endl;
        output  << std::setw(20) << "Offset" 
                << std::setw(20) << "Section" 
                << std::setw(20) << "SymTable Reference" << endl;

        for(auto i=relTable->begin();i!=relTable->end();i++){
            relocationTable_entry r=*i;

            output << std::left 
            << std::setw(20) << r.offset 
            << std::setw(20) << r.section 
            << std::setw(20) << r.symTableReference << endl;
        }
        output<<"\n"<<endl;

        /* CODE */
        for(auto i=mmap.begin();i!=mmap.end();i++){
        string sec=i->first;
        output<<"section:"<<endl;
        output<<sec<<endl;
        vector<char> vektor=mmap[sec];
        int br=0;
        for(int i2=0;i2<vektor.size();i2++) {
            output<<vektor[i2]; 
          /* if(br==8){
                output<<" ";
                br=0;
            }
            br++;   */
        }
        output<<"\n"<<endl;
        output<<"pool:"<<endl;
        
        for(int i=0;i<literalPool.size();i++){
                if(literalPool[i].section==sec){
                    output<<formatLEFourBytes(literalPool[i].literal)<<" ";
                }
        }
        for(int i=0;i<symbolPool.size();i++){
                if(symbolPool[i].section==sec){
                    output<<symbolPool[i].symbol<<" ";
                }
        }
        
        output<<"\n"<<endl;
       }
        output.close();
        
}

void Assembler::makeSmallerOutputFile(string outFile){
    ofstream output;
    output.open(outFile);
    for(auto i=symTable->begin();i!=symTable->end();i++){
            symTable_entry s=*i;           
           
            output << std::left << s.symbolName
            << ":" << s.section
            << ":" << s.symbolId
            << ":" << s.offset 
            << ":" << s.isGlobal 
            << ":" << s.size <<endl;
    }
    output<<"---"<<endl;

    for(auto i=relTable->begin();i!=relTable->end();i++){
            relocationTable_entry r=*i;

            output << std::left << r.offset 
            <<  ":" << r.section 
            <<  ":" << r.symTableReference 
            <<":" << r.symbolId << endl;
        }
        output<<"---"<<endl;
   /*   for(auto i=code->begin();i!=code->end();i++){
            code_entry c=*i;
            output<<"section:\n"<<c.section <<endl;
            
            if(c.bytes.size()==0) cout<<"Bytes is EMPTY"<<endl;
            for (int it = 0; it != c.bytes.size(); it++) {
                
                output<<c.bytes[it];
            }
            output<<"\n"<<endl;
            output<<"pool:"<<endl;
            for(int i=0;i<literalPool.size();i++){
                if(literalPool[i].section==c.section){
                    output<<literalPool[i].literal<<" ";
                }
            }
            for(int i=0;i<symbolPool.size();i++){
                if(symbolPool[i].section==c.section){
                    output<<symbolPool[i].symbol<<" ";
                }
            }
            output<<"\n"<<endl;
        }   */
     for(auto i=mmap.begin();i!=mmap.end();i++){
        string sec=i->first;
        output<<"section:"<<endl;
        output<<sec<<endl;
        vector<char> vektor=mmap[sec];
        for(int i2=0;i2<vektor.size();i2++) {
            output<<vektor[i2]; 
        }
        output<<"\n"<<endl;
        output<<"pool:"<<endl;
        
        for(int i=0;i<literalPool.size();i++){
                if(literalPool[i].section==sec){
                    output<<formatOperandFourBytes(decimalToHex(literalPool[i].literal))<<" ";
                }
        }
        for(int i=0;i<symbolPool.size();i++){
                if(symbolPool[i].section==sec){
                    output<<symbolPool[i].symbol<<" ";
                }
        }
        output<<"\n"<<endl;
       }    
        output<<"END"<<endl;
        output.close();        
}

void Assembler::errorLiteral(){
    cout<<"Error: literal in register indirect addressing should not be larger than 12bit"<<endl;
}




#include "../inc/linker.hpp"
#include "../inc/assembler.hpp"
#include <vector>
#include <string>
#include <list>
#include <algorithm>

using namespace std;

int fileId=0;
void Linker::readFile(string filename)
{
   
    ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    string line;
    istringstream iss(line);
    int num=0; //num of bytes
    while (1)
    { 
        getline(inputFile, line);
        if (line == "---") break;
        vector<string> s = split(line, ':');
        symTable_entry entry = symTable_entry(s[0], s[1], stoi(s[2]), stoi(s[3]), stoi(s[4]), stoi(s[5]), fileId);
        
        symTable_entry ste=findSymbol(entry.symbolName);
        if(ste.symbolName==""){
            if(entry.offset!=-1)
            globalSymTable->push_back(entry);
         
        }
        else{
            
          /*  
            if(sub=="undefined"){
                updateSection(entry.symbolName, entry.section);
            }
            */
        }

    }
    while (1)
    {
        getline(inputFile, line);
        if (line == "---") break;
        vector<string> s = split(line, ':');
        relocationTable_entry entry = relocationTable_entry(stoi(s[0]), s[1], s[2],stoi(s[3]), fileId);
        relTable->push_back(entry);
    }
    string section;
    int instrCount=0;
   
    while (1)
    {
        getline(inputFile, line);
        if (line == "END")
            break;
        if (line == "\n")
            continue;
        if (line == "section:")
        {
            getline(inputFile, line);   
            section=line;
            section_entry entry = section_entry(section, fileId);
            
            sections->push_back(entry);
            getline(inputFile, line);
            vector<char> bytes;
            bytes.assign(line.begin(), line.end());
            instrCount=bytes.size()/8; //svaka instr ima 8 cifara
            num+=bytes.size()/2;  //1 bajt je 2 hex cifre
            
            if(codeMap.find(section+to_string(fileId))!=codeMap.end()){
                codeMap[section+to_string(fileId)].insert(codeMap[section+to_string(fileId)].end(),bytes.begin(), bytes.end());
            }
            else{
                codeMap.insert({section+to_string(fileId), bytes});
                cout<<"inserted section "<<section<<endl;
            }
            
        }
        if (line == "pool:")
        {
            getline(inputFile, line);
            if (line == "\n") continue;
            vector<string> str = split(line, ' ');
            vector<string> emptyStr;
            
            literalPoolMap.insert({section+to_string(fileId), emptyStr});
            for (int i = 0; i < str.size(); i++)
            {
               literal_entry l = literal_entry(hexadecimalToDecimal(str[i]), 256 + i  * 4, section);
               literalPool.push_back(l);
               literalPoolMap[section].push_back(str[i]);
                num+=8;
            }
        }
    }
    cout<<"end of input"<<endl;
    fileId++;
    inputFile.close();
}

 string Linker::formatOperand(string value) {
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

symTable_entry Linker::findSymbol(string symbolName){
    bool exists=0;
    symTable_entry* e;
    symTable_entry s=symTable_entry("");
    for(auto i=globalSymTable->begin();i!=globalSymTable->end();i++){
        if(i->symbolName==symbolName){
            exists=1;
            s.symbolName=symbolName;
            s.offset=i->offset;
            s.isGlobal=i->isGlobal;
            s.section=i->section;
            s.size=i->size;
            break;
        }
    }
    return s;
}

void Linker::updateOffset(string symbolName, int offset){
    for(auto i=globalSymTable->begin();i!=globalSymTable->end();i++){
        if(i->symbolName==symbolName){
            i->offset=offset;
            break;
        }
    }
}

void Linker::updateSection(string symbolName, string section){
    for(auto i=globalSymTable->begin();i!=globalSymTable->end();i++){
        if(i->symbolName==symbolName){
            i->section=section;
            break;
        }
    }
}

void Linker::setSectionStartAddr(int addr, string section){
    for (auto i = sections->begin(); i != sections->end(); i++)
        if (i->sectionName == section)
            i->startingAddr = addr;
}

void Linker::relocateSections(){
    int startAdr = 0;

    for(auto i=sections->begin();i!=sections->end();i++){
        if(i->relocated==0){
            i->startingAddr=startAdr;
            startAdr+=256;
            startAdr+=i->poolSize; // startna adresla sl sekcije je krajnja ove
            i->relocated=1;

            for (auto i2=sections->begin();i2!=sections->end();i2++){
                if (i2->relocated==0 && i2->sectionName==i->sectionName){
                    // ako se neke sekcije isto zovu
                    i2->startingAddr = startAdr; // nadovezujemo ih jednu na drugu
                    startAdr += 256;
                    startAdr += i2->poolSize;
                    i2->relocated = 1;
                }
            }
        }
    }
}

int Linker::getPoolSize(string section){
    int size=0;
    //return literalPoolMap[section].size()*4;
    for(auto i=0;i<literalPool.size();i++){
        if(literalPool[i].section==section)
            size+=4;  //za svaki literal po 4B
    }
    return size;
    
}

void Linker::resolveRelocations(){
    //prolazimo kroz tabelu relok i menjamo na odgovarajucim adresama
    for(auto i=relTable->begin();i!=relTable->end();i++){
 //u sekciju x (reloc tabela) na adresi y (reloc tabela) upisati vrednost koja treba (iz symbol tabele)
        int offset;
        string section;
        for(auto i2=globalSymTable->begin();i2!=globalSymTable->end();i2++){
            //cout<<"sekcija "<<i->section<<endl;
            //cout<<"vrednost koja treba da se upise "<<i2->offset<<endl;
            string operand=formatOperandFourBytes(decimalToHex(i2->offset));
            //if(i2->isGlobal==0){
            if(i->symTableReference==i2->symbolName) {    
            if(i->offset<256){
                vector<char> v=codeMap[i->section];
                v.assign(codeMap[i->section].begin(), codeMap[i->section].end());
                int index=i2->offset/4;
                int cnt=0;  //broji do 8, treba da se prepravi 8 cifara
                for(char ch : operand){
                    v[index+cnt]=ch;
                    cnt++;
                }  
            }
            else{ //ako je vece od 256 znaci da je u bazenu
                int pom=i->offset-256;
                //cout<<"i->offset "<<i->offset<<endl;
                //cout<<"pom "<<pom<<endl;
                int index=pom/4;
                //cout<<"index "<<index<<endl;        
                vector<string> vect=literalPoolMap[i->section];
                vect.assign(literalPoolMap[i->section].begin(), literalPoolMap[i->section].end());
                vect[index]=operand;
                //cout<<"operand--------->"<<operand<<endl;
                literalPoolMap[i->section].assign(vect.begin(), vect.end());
                } 
            }
        }
    
    }    
}

void Linker::resolveGlobalSymbols(){
    //rasporedili smo sekcije
    //upisemo adresu simbola + adresu pocetka sekcije
    for(auto i=relTable->begin();i!=relTable->end();i++){
        for(auto i2=globalSymTable->begin();i2!=globalSymTable->end();i2++){
            if(i->symTableReference==i2->symbolName){
                if(i->section!=i2->section && i2->isGlobal==0) {
                    cout<<"Error: there is no (global) definition of symbol."<<endl;
                }
                int startingAdr=0;
                int startingAdr2=0;
                //cout<<"symbol: "<<i->symTableReference<<endl;
                for(auto i3=sections->begin();i3!=sections->end();i3++){
                    if(i3->sectionName==i2->section && i3->fileId==i2->fileId) startingAdr2=i3->startingAddr;
                    if(i3->sectionName==i->section && i3->fileId==i->fileId) startingAdr=i3->startingAddr;

                }
                //cout<<"section where is the defintion: "<<i2->section<<endl;
                //cout<<"section that should be changed: "<<i->section<<endl;
                string operand=formatOperandFourBytes(decimalToHex(i2->offset + startingAdr2));
                //cout<<"operand--------->"<<operand<<endl;
                int adrToChange=i->offset + startingAdr; 
                
                vector<char> v;
                if((i->offset+startingAdr)%8==0){
                    string adrToChangeStr=formatOperandFourBytes(decimalToHex(adrToChange));
                    //cout<<"adr to change->"<<formatOperandFourBytes(decimalToHex(adrToChange))<<endl;
                    v.assign(finalCodeMap[adrToChangeStr].begin(), finalCodeMap[adrToChangeStr].end());
                    int cnt=0;
                    for(char ch : operand){
                    v[cnt]=ch;
                    cnt++;
                    } 
                    finalCodeMap[adrToChangeStr].assign(v.begin(), v.end());
                }   
                else{
                    adrToChange-=4;
                    string adrToChangeStr=formatOperandFourBytes(decimalToHex(adrToChange));
                    //cout<<"adr to change->"<<formatOperandFourBytes(decimalToHex(adrToChange))<<endl;
                    v.assign(finalCodeMap[adrToChangeStr].begin(), finalCodeMap[adrToChangeStr].end());
                    int cnt=8;
                    for(char ch : operand){
                    v[cnt]=ch;
                    cnt++;
                    } 
                    finalCodeMap[adrToChangeStr].assign(v.begin(), v.end());
                }
                
            }
        }
    }
    cout<<"PREPRAVLJENO."<<endl;

}
//int cntOfSameSections=0;
void Linker::relocateSectionsWithPlace(list<section_places> *places)
{
    for(auto i=places->begin(); i!=places->end();i++){
        int startAdr = i->address;

        for(auto i2=sections->begin();i2!=sections->end();i2++){
            if(i2->relocated==0 && i->section==i2->sectionName) { // nadjemo u listi sekcija onu koja je zadata sa place
                i2->startingAddr=startAdr; // i podesimo njenu adresu
                i2->relocated=1;
                startAdr+=i2->size;
                startAdr+=getLitNumInSection(i2->sectionName)*4;
                i->size+=i2->size;
                i->size+=getLitNumInSection(i2->sectionName)*4;

                for(auto i3=sections->begin();i3!=sections->end();i3++){
                    if(i3->relocated==0 && i3->sectionName==i2->sectionName) // ako se neka druga isto zove nadovezujemo je
                    {
                        i3->startingAddr=startAdr;
                        i3->relocated=1;
                        startAdr+=i3->size;
                        startAdr += getLitNumInSection(i3->sectionName) * 4;
                        i->size+=i2->size;
                        i->size+=getLitNumInSection(i2->sectionName)*4;
                    }
                }
            }
        }
    }
    //da li se preklapaju
    bool overlap=false;
    
    int maxAdr=places->begin()->address + places->begin()->size;

    for (auto i=places->begin();i!=places->end();i++){
        for (auto i2=places->begin();i2!=places->end(); i2++){
            if (i->section!=i2->section){
                if ((i->address<= i2->address && i->address+ i->size >= i2->address) ||
                    (i->address >= i2->address && i2->address + i2->size >= i->address))
                    overlap = true;
                else{    
                    if (i2->address > i->address)
                        maxAdr = i2->address + i2->size + getLitNumInSection(i2->section)*4;
                    else
                        maxAdr = i->address + i->size + getLitNumInSection(i->section)*4;
               }
            }
        }
    }
   /* if (overlap){
        cout<<"Error: sections are  overlapping "<<endl;
    }
    */

    for (auto i=sections->begin();i!=sections->end();i++){
        if(i->relocated== 0){
            i->startingAddr=maxAdr;
            i->relocated=1;
            maxAdr+=i->size;
            maxAdr+=getLitNumInSection(i->sectionName)*4;

            for(auto i2=sections->begin();i2!=sections->end();i2++){
                if (i2->relocated==0 && i2->sectionName==i->sectionName){  //ako su sekcije istog naziva
                    i2->startingAddr=maxAdr;
                    i2->relocated=1;
                    maxAdr+=i2->size;
                    maxAdr+=getLitNumInSection(i2->sectionName);
                }
            }
        }
    }
}

vector<string> Linker::split(const string &str, char delimiter)
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

string Linker::decimalToHex(int decimalNumber){
        std::stringstream ss;
        ss << std::hex << decimalNumber;
        string s=ss.str();
        return s;
}

int Linker::hexadecimalToDecimal(string hexVal) { 
    int len = hexVal.size(); 
  
    // Initializing base value to 1, 
    // i.e 16^0 
    int base = 1; 
    int dec_val = 0; 
  
    // Extracting characters as digits 
    // from last character 
    for (int i = len - 1; i >= 0; i--) { 
        // If character lies in '0'-'9', 
        // converting it to integral 0-9 
        // by subtracting 48 from ASCII value 
        if (hexVal[i] >= '0' && hexVal[i] <= '9') { 
            dec_val += (int(hexVal[i]) - 48) * base; 
  
            // incrementing base by power 
            base = base * 16; 
        } 
  
        // If character lies in 'A'-'F' , converting 
        // it to integral 10 - 15 by subtracting 55 
        // from ASCII value 
        else if (hexVal[i] >= 'A' && hexVal[i] <= 'F') { 
            dec_val += (int(hexVal[i]) - 55) * base; 
  
            // Incrementing base by power 
            base = base * 16; 
        } 
    } 
    return dec_val; 
} 

string Linker::formatOperandFourBytes(string value) {
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

string Linker::formatFourDigits(string value){
        string val;
        if(value.length() == 1){
            val = "000" + value;
        }
        else if(value.length() == 2){
            val = "00" + value;
        }
        else if(value.length() == 3){
            val = "0" + value;
        }
        else{
            val = value;
        }
        return val;
} 
int Linker::getLitNumInSection(string section){
        int num=0;
        return literalPoolMap[section].size();
      /* for(int i=0;i<literalPool.size();i++)
            if(literalPool[i].section==section) num++;
        return num;
        */
        
    }

bool Linker::sectionExists(string section){
    bool exists=false;
    for(auto i=sections->begin();i!=sections->end();i++){
        if(i->sectionName==section){
            exists=true;
            break;
        }
    }
    return exists;
}

bool Linker::sectionsSorter(const section_entry &lhs, const section_entry &rhs){
      return lhs.startingAddr < rhs.startingAddr;
}

void Linker::printGlobalSymTable(){
    ofstream output;
    output.open("globalSymTable.txt");
    for(auto i=globalSymTable->begin();i!=globalSymTable->end();i++){
        output<<"section "<<i->section<<" symbol "<<i->symbolName<<" offset "<<i->offset<<endl;
    }
}
void Linker::printRelocTable(){
    ofstream output;
    output.open("relocTable.txt");
    for(auto i=relTable->begin();i!=relTable->end();i++){
        output<<"section "<<i->section<<" symbol "<<i->symTableReference<<" offset "<<i->offset<<endl;
    }

}
void Linker::sortSections()
{
  sections->sort(Linker::sectionsSorter);
  cout << "Sorted sections: " << endl;
  for (auto i = sections->begin(); i != sections->end(); i++)
    cout << i->sectionName <<"file id:"<<i->fileId <<" start: " << decimalToHex(i->startingAddr) << endl;
  /*
int adr=0;
string section;
vector<char> bytes;
for(auto i=sections->begin();i!=sections->end();i++){
  section=i->sectionName;
  for(auto i=codeMap.begin();i!=codeMap.end();i++){
      if(section==i->first){
          bytes=i->second;
          sortedCodeMap.insert({section, bytes});
          cout<<"inserted "<<section<<endl;
      }
  }
}
for(auto i=sortedCodeMap.begin();i!=sortedCodeMap.end();i++){
  cout<<"sorted code map "<<i->first<<endl;
}
*/
}


void Linker::makeOutputMap(){
    sortSections();
    for(auto i=sections->begin();i!=sections->end();i++){
        //cout<<"section "<<i->sectionName<<endl;
        int adr=i->startingAddr;
        string section=i->sectionName;
        
        //cout<<"starting adr "<<decimalToHex(i->startingAddr)<<" of section "<<i->sectionName<<endl;
        for(auto i2=codeMap.begin();i2!=codeMap.end();i2++){
        if(i->sectionName+to_string(i->fileId)==i2->first){
            //cout<<i->sectionName+to_string(i->fileId)<<endl;
            int j=0;
            int digitsNum=0;
            vector<char>bytes;
            while (j!=i2->second.size()){ 
                bytes.push_back(i2->second[j]);
                digitsNum++;
                j++;

            if(digitsNum==16){
                string adrStr=formatFourDigits(decimalToHex(adr));
                finalCodeMap.insert({adrStr, bytes});
                digitsNum=0;
                adr+=8;
                bytes.clear();
            }
            else if(j==i2->second.size()){
                string adrStr=formatFourDigits(decimalToHex(adr));
                finalCodeMap.insert({adrStr, bytes});
                digitsNum=0;
                adr+=8;
                bytes.clear();
            }    
           }
        }
        }
        int litNum=0;
        vector<char> l;
        vector<string> strings=literalPoolMap[section];
        strings.assign(literalPoolMap[section].begin(), literalPoolMap[section].end());
        //cout<<"literal pool map size of section "<<section<<":"<<literalPoolMap[section].size()<<endl;
        int count=0;
        for(int k=0;k<strings.size();k++){
            int adrPool=i->startingAddr + 256-4 + k*4;
            string adrPoolStr=formatOperandFourBytes(decimalToHex(adrPool));
            string litValue=strings[k];
            count++;
            l.insert(l.end(), litValue.begin(), litValue.end());
            litNum++;
            
            if(litNum==2){
                finalCodeMap.insert({adrPoolStr, l});
                litNum=0;
                l.clear();
            }
            else if(k==strings.size()-1){
                int addrPool=uint(stoul(adrPoolStr,0,16))+4;
                adrPoolStr=formatOperandFourBytes(decimalToHex(addrPool));
                //cout<<"adr bazena: "<<adrPoolStr<<endl;
                finalCodeMap.insert({adrPoolStr, l});
                //cout<<"inserted at adr "<<adrPoolStr<<endl;
                litNum=0;
                l.clear();
            }
        }
      
    }
}

void Linker::printLiteralPoolMap(string outp){
    ofstream output;
    output.open(outp);
    for(auto i=literalPoolMap.begin();i!=literalPoolMap.end();i++){
        output<<i->first<<":"<<endl;
    for(int j=0;j<i->second.size();j++){
        output<<i->second[j]<<endl;
    }
    }
}

void Linker::printCodeMap(string outp)
{
    ofstream output;
    output.open(outp);
    for(auto i=codeMap.begin();i!=codeMap.end();i++){
        output<<i->first<<":"<<endl;
    for(int j=0;j<i->second.size();j++){
        output<<i->second[j]<<endl;
    }
    }
}
void Linker::makeOutputFile(string outFile)
{
    ofstream output;
    ofstream output2;
    output.open(outFile);
  /* output<<"Nesortirana"<<endl;
    for(auto i=codeMap.begin();i!=codeMap.end();i++){
         output <<  i->first <<":";
        for(auto i2=0;i2!=i->second.size();i2++){
            output << i->second[i2];
        }
        output<<"\n";
    }
*/

    for(auto i=finalCodeMap.begin();i!=finalCodeMap.end();i++){
        output<< i->first << ":";
        //cout<<"ADRESA "<<i->first<<endl;
        for(auto i2=0;i2!=i->second.size();i2++){
            //cout<<i->second[i2]<<endl;
            output << i->second[i2];
            if(i2%2 && i2!=0) output<<" ";
        }
        output<<"\n";
    }
    output<<"END"<<endl;

    output.close();

    output2.open("linkerOutput.txt");
    for(auto i=finalCodeMap.begin();i!=finalCodeMap.end();i++){
        output2<< i->first << ":";
        //cout<<"ADRESA "<<i->first<<endl;
        for(auto i2=0;i2!=i->second.size();i2++){
            //cout<<i->second[i2]<<endl;
            output2 << i->second[i2];
            if(i2%2 && i2!=0) output2<<" ";
        }
        output2<<"\n";
    }
    output2.close();
}
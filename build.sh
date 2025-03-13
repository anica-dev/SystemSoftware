cd misc
bison -d bison.y
flex flex.l
cd ..
g++ src/lexer.cpp src/parser.cpp src/assembler.cpp src/mainAs.cpp -o asembler
g++ src/linker.cpp src/mainLinker.cpp -o linker
g++ src/emulator.cpp src/mainEmulator.cpp -o emulator

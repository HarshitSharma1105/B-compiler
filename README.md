# B-compiler
Compiler for the dead language B
# Build Process
For compiling the compiler : 
    -g++ main.cpp -o main.exe 


For running the compiler with or without debug info :  
    ./main.exe test.b debug/nodebug;

    
For running the generated MIPS Assemlby:

QtSpim.exe -file output.asm  (Opens a QtSpim window, have to click run button to execute)
OR
java -jar Mars4_5.jar sm output.asm 



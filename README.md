# B-compiler
Compiler for the dead language B which produces MIPS or X86_64 Assembly
# Build Process
For compiling the compiler just run make
If you have already built the compiler once and want to run it, just do make run

Flags :

File name can be provided through file_name 

Mode of running (debug/release) can be provided through mode 

Ex Command:
make file_name="examples/example001/hello.b" mode="nodebug"

The compiler will automatically call the respective assembly pipelines for you .

For running the generated Assembly:
If MIPS:
    You need Java installed 
If X86_64:
    You just need gcc 





# B-compiler
Compiler for the dead language B which produces MIPS Assembly(for now)
# Build Process
For compiling the compiler just run make
If you have already built the compiler once and want to run it,
just do make run
Flags :

File name can be provided through file_name 

Mode of running (debug/release) can be provided through mode 

Ex Command:
make file_name="examples/example001/hello.b" mode="nodebug"

For running the generated MIPS Assembly:
You need Java installed and the Mars4_5.jar file provided in this repo. The compiler will automatically call the pipeline for you .





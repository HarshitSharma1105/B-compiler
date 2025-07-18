#include "generator.cpp"



int main(int argc,char* argv[])
{
   
    std::string b_sourcecode,path=argv[1];
    bool debugging=(std::string)argv[2]=="debug";
    {
        std::stringstream contents_stream;
        std::fstream input(path, std::ios::in);
        contents_stream << input.rdbuf();
        b_sourcecode = contents_stream.str();
    }
    std::string finalb_sourcecode=preprocessor(path,b_sourcecode);
    Tokenizer tokenizer(finalb_sourcecode);
    std::vector<Token> tokens=tokenizer.tokenize();
    IREmittor iremittor(tokens);
    std::vector<Op> ops=iremittor.EmitIR();
    Generator_x86_64 generator(ops);
    std::string assembly_sourcecode=generator.generate();
    debug(ops);
    std::cout << assembly_sourcecode;
    if(debugging)
    {
        debug(tokens);
        std::cout << finalb_sourcecode << std::endl;
        std::cout << assembly_sourcecode << std::endl; 
    }
    {
        std::ofstream outFile("output.asm");  
        outFile << assembly_sourcecode;
    }
    std::string command = "fasm output.asm";
    system(command.c_str());
    command = "cc -no-pie output.o -o output";
    system(command.c_str());
    command = "./output";
    system(command.c_str());
    if(!debugging)
    {
        system("rm output.asm");
        system("rm output.o");
    }
    return 0;
}
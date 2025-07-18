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
    Generator generator(tokens);
    IREmittor iremittor(tokens);
    std::vector<Op> ops=iremittor.EmitIR();
    debug(ops);
    std::string assembly_sourcecode=generator.generate();
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
    std::string command = "java -jar assemblers/Mars4_5.jar sm output.asm";
    system(command.c_str());
    if(!debugging)system("rm output.asm");
    return 0;
}
#include<fstream>
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
    b_sourcecode.push_back('\0');
    Tokenizer tokenizer(b_sourcecode);
    std::vector<Token> tokens=tokenizer.tokenize();
    Generator generator(tokens);
    std::string assembly_sourcecode=generator.generate();
    if(debugging)
    {
        debug(tokens);
        std::cout << b_sourcecode << std::endl;
        std::cout << assembly_sourcecode << std::endl; 
    }
    {
        std::ofstream outFile("output.asm");  // create and open the file
        outFile << assembly_sourcecode;
    }
    return 0;
}
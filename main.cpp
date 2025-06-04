#include<fstream>
#include "generator.cpp"


void open_file(const std::string& path,std::string& b_sourcecode)
{
    {
        std::stringstream contents_stream;
        std::fstream input(path, std::ios::in);
        contents_stream << input.rdbuf();
        b_sourcecode += contents_stream.str();
    }
}


int main(int argc,char* argv[])
{
    std::string b_sourcecode,path=argv[1];
    bool debugging=(std::string)argv[2]=="debug";
    open_file(path,b_sourcecode);
    path.clear();
    int index=0;
    while(b_sourcecode[index++]=='#'){
        while(b_sourcecode[index]!='\"')
        {   
            if(!std::isspace(b_sourcecode[index])) path.push_back(b_sourcecode[index]);
            index++;
        }
        if(path!="include")
        {
            std::cerr << "wrong declaration\n" << path.back();
            exit(EXIT_FAILURE);
        }
        path.clear();
        index++;
        while(b_sourcecode[index]!='\"')
        {   
            path.push_back(b_sourcecode[index++]);
        }
        while(true)
        {
            if(std::isspace(b_sourcecode[index]))index++;
            else break;
        }
        std::cout << path << "\n";
        open_file(path,b_sourcecode);
        path.clear();
    }
    std::cout << b_sourcecode << std::endl;
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
        std::ofstream outFile("output.asm");  
        outFile << assembly_sourcecode;
    }
    std::string command = "java -jar Mars4_5.jar sm output.asm";
    system(command.c_str());
    return 0;
}
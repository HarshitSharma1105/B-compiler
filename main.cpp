#include<fstream>
#include "generator.cpp"


void open_file(const std::string& path,std::string& b_sourcecode)
{
    {
        std::stringstream contents_stream;
        std::fstream input(path, std::ios::in);
        contents_stream << input.rdbuf();
        b_sourcecode += contents_stream.str();
        b_sourcecode.push_back('\n');
    }
}


int main(int argc,char* argv[])
{
    std::string b_sourcecode,finalb_sourcecode,path=argv[1];
    bool debugging=(std::string)argv[2]=="debug";
    open_file(path,b_sourcecode);
    std::string rel_path;
    int index=0;
    while(b_sourcecode[index]=='#'){
        index++;
        while(b_sourcecode[index]!='\"')
        {   
            if(!std::isspace(b_sourcecode[index])) rel_path.push_back(b_sourcecode[index]);
            index++;
        }
        if(rel_path!="include")
        {
            std::cerr << "wrong declaration\n" << rel_path.back();
            exit(EXIT_FAILURE);
        }
        rel_path.clear();
        index++;//consume the starting double qoute
        while(b_sourcecode[index]!='\"')
        {   
            rel_path.push_back(b_sourcecode[index++]);
        }
        index++;//consume the ending double qoute
        open_file(rel_path,finalb_sourcecode);
        rel_path.clear();
        while(true)
        {
            if(std::isspace(b_sourcecode[index]))index++;
            else break;
        }
    }
    finalb_sourcecode+=b_sourcecode.substr(index,b_sourcecode.size()-index+1);
    finalb_sourcecode.push_back('\0');
    Tokenizer tokenizer(finalb_sourcecode);
    std::vector<Token> tokens=tokenizer.tokenize();
    Generator generator(tokens);
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
    std::string command = "java -jar Mars4_5.jar sm output.asm";
    system(command.c_str());
    return 0;
}
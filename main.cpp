#include<fstream>
#include "tokenizer.cpp"



int main(int argc,char* argv[])
{
    std::string contents,path=argv[1];
    {
        std::stringstream contents_stream;
        std::fstream input(path, std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }
    contents.push_back('\0');
    Tokenizer tokenizer(contents);
    std::vector<Token> tokens=tokenizer.tokenize();
    debug(tokens);
    std::cout << contents;
    return 0;
}
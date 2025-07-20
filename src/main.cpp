#include "generator.cpp"



int main(int argc,char* argv[])
{
   
    std::string b_sourcecode,path=argv[1],parent_path=std::filesystem::absolute(path).parent_path().string(),trash_path=parent_path+"/trash";;
    bool debugging=(std::string)argv[2]=="debug";
    {
        std::stringstream contents_stream;
        std::fstream input(path, std::ios::in);
        if (!input.is_open()) 
        {
            std::cerr << "Failed to open file   " << path << "\n";
            exit(EXIT_FAILURE);
        }
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
    if(debugging)
    {
        debug(tokens);
        debug(ops);
        std::cout << finalb_sourcecode << std::endl;
        std::cout << assembly_sourcecode << std::endl; 
    }
    std::string command = "mkdir " + trash_path;
    system(command.c_str());
    {
        std::ofstream outFile(trash_path+ "/output.asm");  
        outFile << assembly_sourcecode;
    }
    command = "fasm " + trash_path + "/output.asm";
    system(command.c_str());
    command = "cc -no-pie " + trash_path +"/output.o -o "+"builds/output";
    system(command.c_str());
    command = "builds/output";
    system(command.c_str());
    if(!debugging)
    {
        system(("rm -rf "+trash_path).c_str());
    }
    return 0;
}
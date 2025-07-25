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
    //TODO Fix this target situation with something!!!
    //Generator_Mips generator(ops);
    Generator_x86_64 generator(ops);
    std::string assembly_sourcecode=generator.generate();
    if(debugging)debug(ops);
    std::string command = "mkdir " + trash_path;//TODO: Use smth like mkdir_if_not_exist
    system(command.c_str());
    {
        std::ofstream outFile(trash_path+ "/output.asm");  
        outFile << assembly_sourcecode;
    }
    command = "assemblers/fasm " + trash_path + "/output.asm";
    system(command.c_str());
    command = "cc -no-pie " + trash_path +"/output.o -o "+"builds/output";
    system(command.c_str());
    command = "builds/output";
    system(command.c_str());
    // command="java -jar assemblers/Mars4_5.jar sm " + trash_path +"/output.asm";
    // system(command.c_str());
    if(!debugging)system(("rm -rf "+trash_path).c_str());
    return 0;
}
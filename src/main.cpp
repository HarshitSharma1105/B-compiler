#include<Runner.h>


int main(int argc,char* argv[])
{
    if(argc<4)
    {
        std::cerr << "Incorrect Usage\n";
        exit(EXIT_FAILURE);
    }
    std::string b_sourcecode,path=argv[1],parent_path=std::filesystem::absolute(path).parent_path().string();
    bool debugging=(std::string)argv[2]=="debug";
    open_file(path,b_sourcecode);

    std::string finalb_sourcecode=preprocessor(path,b_sourcecode);

    Tokenizer tokenizer(finalb_sourcecode);
    std::vector<Token> tokens=tokenizer.tokenize();

    IREmittor iremittor(tokens);
    std::vector<Op> ops=iremittor.EmitIR();

    std::string target_lang=argv[3];
    Runner runner(target_lang,parent_path);
    runner.compile(ops);
    runner.run();
    if(debugging)debug(ops);
    else system(("rm -rf "+parent_path+"/trash/").c_str());
    return 0;
}

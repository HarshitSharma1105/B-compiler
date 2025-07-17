#include<string>
#include<filesystem>
#include<sstream>
#include<fstream>
#include<iostream>


void open_file(const std::string& path,std::string& finalb_sourcecode)
{
    if(!path.empty())
    {
        std::stringstream contents_stream;
        std::fstream input(path, std::ios::in);
        if (!input.is_open()) 
        {
            std::cerr << "Failed to open file   " << path << "\n";
            exit(EXIT_FAILURE);
        }
        contents_stream << input.rdbuf();
        finalb_sourcecode += contents_stream.str();
        finalb_sourcecode.push_back('\n');
    }
}





std::string preprocessor(std::string path,const std::string& b_sourcecode)
{

    std::string finalb_sourcecode;
    path=std::filesystem::absolute(path).parent_path().string();
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
        rel_path="/";
        index++;//consume the starting double qoute
        while(b_sourcecode[index]!='\"')
        {   
            rel_path.push_back(b_sourcecode[index++]);
        }
        index++;//consume the ending double qoute
        rel_path=path+rel_path;
        std::string temp;
        open_file(rel_path,temp);
        finalb_sourcecode+=preprocessor(rel_path,temp);
        rel_path.clear();
        while(std::isspace(b_sourcecode[index]))
        {
            index++;
        }
    }
    finalb_sourcecode+=b_sourcecode.substr(index,b_sourcecode.size()-index+1);
    return finalb_sourcecode;
}

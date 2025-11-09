#pragma once


#include<InterRepr.h>



class Generator_Mips
{
public:
    Generator_Mips(const Compiler &compiler);
    std::string generate();

private:
    void generate_stdlib();
    Compiler compiler;
    std::stringstream textstream;
    std::unordered_set<std::string> extrns;
};
#pragma once

#include<InterRepr.h>




class Generator_x86_64
{
public:
    Generator_x86_64(const Compiler& compiler);
    std::string generate();
    
private:
    void generate_func(const Func& func);
    void generate_function_epilogue();
    void generate_function_prologue(const Func& func);
    void generate_stdlib();
    Compiler compiler;
    std::stringstream textstream;
    std::unordered_set<std::string> extrns;
};

#pragma once

#include<InterRepr.h>

namespace x86_64
{
    inline std::array<std::string,6> regs = {"rdi","rsi","rdx","rcx","r8","r9"};
    struct ArgVisitor
    {
        std::stringstream& stream;
        std::vector<std::string>& globals;
        std::vector<std::pair<std::string,size_t>>& arrays;
        void operator()(const Var& var);
        void operator()(const Literal& literal);
        void operator()(const DataOffset& data);
        void operator()(const FuncResult& funcresult);
        void operator()(const Ref& ref);
    };
    struct Visitor
    {
        std::stringstream& stream;
        std::vector<std::string>& globals;
        std::vector<std::pair<std::string,size_t>>& arrays;
        ArgVisitor argvisitor{stream,globals,arrays};
        void operator()(const UnOp& unop);
        void operator()(const BinOp& binop);
        void operator()(const DataSection& data);
        void operator()(const Funcall& funcall); 
        void operator()(const ReturnValue& retval);
        void operator()(const Label& label);
        void operator()(const JmpIfZero& jz);
        void operator()(const Jmp& jmp);
        void operator()(const Store& store);
        void operator()(const Asm& assembly);
    };
};



class Generator_x86_64
{
public:
    Generator_x86_64(const Compiler& compiler);
    std::string generate();
    
private:
    void generate_func(const Func& func);
    void generate_function_epilogue(const Func& func);
    void generate_function_prologue(const Func& func);
    void generate_stdlib();
    Compiler compiler;
    std::stringstream textstream;
    x86_64::Visitor visitor{textstream,compiler.globals,compiler.arrays};
    bool is_main;
};

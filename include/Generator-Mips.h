#pragma once


#include<InterRepr.h>
namespace Mips
{
    inline std::array<std::string,4> regs = {"$a0","$a1","$a2","$a3"};
    struct ArgVisitor
    {
        std::stringstream& stream;
        void operator()(const Var& var);
        void operator()(const Literal& literal);
        void operator()(const DataOffset& data);
        void operator()(const FuncResult& funcresult);
        void operator()(const Ref& ref);
        void operator()(const NoArg& noarg);
    };
    struct Visitor
    {
        std::stringstream& stream;
        ArgVisitor argvisitor{stream};
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


class Generator_Mips
{
public:
    Generator_Mips(const Compiler &compiler);
    std::string generate();

private:
    void generate_func(const Func& func);
    void generate_function_epilogue();
    void generate_function_prologue(const Func& func);
    void generate_stdlib();
    Compiler compiler;
    Mips::Visitor visitor{textstream};
    std::stringstream textstream;
    bool is_main=false;
};
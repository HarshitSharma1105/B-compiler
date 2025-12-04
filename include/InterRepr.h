#pragma once

#include<Tokenizer.h>


#include<variant>
#include<unordered_set>
#include<algorithm>
#include<stack>

struct Variable{
    std::string var_name;
    size_t index;
};

struct Var{
    size_t index;
};

struct Ref{
    size_t index;
};

struct Literal{
   size_t literal;
};

struct DataOffset{
    size_t start;
};

struct FuncResult{
    std::string func_name;
};


typedef std::variant<Var,Ref,Literal,DataOffset,FuncResult> Arg;

struct AutoAssign{
    size_t index;
    Arg arg;
};


struct UnOp{
    size_t index;
    Arg arg;
    Tokentype type;
};

struct BinOp{
    size_t index;
    Arg lhs,rhs;
    Tokentype type;
};

struct Funcall{
    std::string name;
    std::vector<Arg> args;
};


struct Store{
    size_t index;
    Arg val;
};

struct DataSection{
    std::string concatedstrings;
};

struct ReturnValue{
    Arg arg;
};

struct Label{
    size_t idx;
};

struct JmpIfZero{
    Arg arg;
    size_t idx;
};

struct Jmp{
    size_t idx;
};

typedef std::variant<AutoAssign,UnOp,BinOp,Funcall,DataSection,ReturnValue,JmpIfZero,Jmp,Label,Store> Op;

typedef std::vector<Op> Ops;

struct Func{
    Ops function_body;
    std::string function_name;
    size_t max_vars_count,num_args;
};


struct Compiler{
    std::vector<Func> functions;
    std::string data_section;
    std::vector<std::string> extrns;
};


void debug(const Ops& ops);


class IREmittor
{
public:
    IREmittor(const std::vector<Token> &tokens);
    Compiler   EmitIR();



private:
    size_t get_var_index(const std::string& name);
    void compile_prog();
    void compile_func_body(Ops& ops);
    bool compile_while_loops(Ops& ops);
    bool compile_return(Ops& ops);
    void compile_stmt(Ops& ops);
    bool autovar_dec(Ops& ops);
    bool compile_extrn(Ops& ops);
    bool compile_branch(Ops& ops);
    void compile_block(Ops& ops);
    bool compile_scope(Ops& ops);
    Arg compile_expression(int precedence,Ops& ops);
    Arg compile_primary_expression(Ops& ops);

    std::optional<Token> peek(int offset=0);
    Token consume();
    Token try_consume(const Tokentype& type, const std::string& err_msg);
    bool try_consume(const Tokentype& type);
    bool try_peek(const std::vector<Tokentype>& types,int offset=0);
    bool try_peek(const Tokentype& type,int offset=0);
    Compiler compiler;
    std::vector<Token> tokens;
    int token_index=0;
    size_t data_offset=0;
    size_t vars_count=0;
    size_t max_vars_count=0;
    size_t labels_count=0;
    std::unordered_set<std::string> funcs;
    std::vector<std::string> extrns;
    std::vector<Variable> vars;
    std::stringstream datastring;
    bool is_main_func_present=false;
};

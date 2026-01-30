#pragma once

#include<Tokenizer.h>


#include<variant>
#include<algorithm>
#include<array>
#include<unordered_set>


enum Storage{
    Auto,
    Global,
    Array
};

struct Var{
    size_t index;
    Storage type;
    std::string var_name;
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

struct NoArg{};

typedef std::variant<Var,Ref,Literal,DataOffset,FuncResult,NoArg> Arg;

struct UnOp{
    size_t index;
    Arg arg;
    Tokentype type;
};

struct BinOp{
    Var var;
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

struct Asm{
    std::string asm_code;
};

typedef std::variant<UnOp,BinOp,Funcall,DataSection,ReturnValue,JmpIfZero,Jmp,Label,Store,Asm> Op;

typedef std::vector<Op> Ops;

struct Func{
    Ops function_body;
    std::string function_name;
    size_t max_vars_count,num_args,func_flags;
    std::vector<size_t> default_args;
};

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

struct Compiler{
    std::vector<Func> functions;
    std::string data_section;
    std::vector<std::string> extrns;
    std::vector<std::pair<std::string,size_t>> arrays,globals;
};
void debug(const Compiler& compiler);


enum Flag
{
    AsmFunc = 1 << 1
};

void debug(const Ops& ops);


class IREmittor
{
public:
    IREmittor(const std::vector<Token> &tokens);
    Compiler   EmitIR();



public:
    Var get_var(const std::string& name);
    void compile_prog();
    void compile_func_body(Ops& ops);
    bool compile_while_loops(Ops& ops);
    bool compile_for_loops(Ops& ops);
    bool compile_return(Ops& ops);
    void compile_stmt(Ops& ops);
    bool autovar_dec(Ops& ops);
    bool compile_extrn();
    bool compile_branch(Ops& ops);
    void compile_block(Ops& ops);
    bool compile_scope(Ops& ops);
    bool compile_asm(Ops& ops);
    bool compile_switch(Ops& ops);
    Arg compile_expression(int precedence,Ops& ops);
    Arg compile_prim_expr(Ops& ops);
    Arg compile_primary_expression(Ops& ops);
    Tokentype conv(const Tokentype& type);
    std::optional<Token> peek(int offset=0);
    Token consume();
    Token try_consume(const Tokentype& type, const std::string& err_msg);
    Token try_consume(const Tokentype& type);
    bool try_peek(const std::vector<Tokentype>& types,int offset=0);
    bool try_peek(const Tokentype& type,int offset=0);
    Compiler compiler{};
    std::vector<Token> tokens;
    int token_index=0;
    size_t data_offset=0;
    size_t vars_count=0;
    size_t max_vars_count=0;
    size_t labels_count=0;
    std::vector<Var> vars;
    std::stringstream datastring;
    std::unordered_set<std::string> functions;
    bool is_main_func_present=false;
};

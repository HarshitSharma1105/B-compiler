#pragma once

#include<Tokenizer.h>


#include<variant>
#include<unordered_map>
#include<unordered_set>
#include<algorithm>
#include<stack>

struct Variable{
    std::string var_name;
    size_t index;
};




struct AutoVar{
    size_t count; 
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


struct ExtrnDecl{
    std::string name;
};


enum UnOpType{
    Negate,
    Not
};


struct UnOp{
    size_t index;
    Arg arg;
    UnOpType type;
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


struct FuncDecl{
    std::string name;
    size_t count;
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


struct JmpIfZero{
    Arg arg;
    size_t idx;
};

struct Jmp{
    size_t idx;
};

struct JmpInfo{
    size_t skip_idx,jmp_idx;
};

enum ScopeType{
    Global,
    Function,
    Local,
    Loop,
    If_,
    Else_
};

struct Scope{
    ScopeType type;
    std::string scope_name;
    size_t vars_count,vars_size;
    JmpInfo info;
};

struct ScopeBegin{
    std::string name;
    ScopeType type;
};
struct ScopeClose{
    std::string name;
    ScopeType type;
};

typedef std::variant<AutoVar,AutoAssign,UnOp,BinOp,ExtrnDecl,Funcall,FuncDecl,
    ScopeBegin,ScopeClose,DataSection,ReturnValue,JmpIfZero,Jmp,Store> Op;





void debug(const std::vector<Op>& ops);


class IREmittor
{
public:
    IREmittor(const std::vector<Token> &tokens);
    std::vector<Op>   EmitIR();



private:
    size_t get_var_index(const std::string& name);

    void compile_statement();
    bool compile_while_loops();
    bool compile_return();
    bool compile_stmt();
    bool scope_open();
    bool scope_end();
    bool autovar_dec();
    bool compile_extrn();
    bool compile_funcdecl();
    bool compile_if();
    bool compile_else();

    Arg compile_expression(int precedence);
    Arg compile_primary_expression();

    std::optional<Token> peek(int offset=0);
    Token consume();
    Token try_consume(const Tokentype& type, const std::string& err_msg);
    bool try_consume(const Tokentype& type);
    bool try_peek(const std::vector<Tokentype>& types,int offset=0);
    bool try_peek(const Tokentype& type,int offset=0);
    
    std::vector<Op> ops;
    std::vector<Token> tokens;
    std::stack<Scope> scopes;
    int token_index=0;
    size_t data_offset=0;
    size_t vars_count=0;
    std::unordered_set<std::string> extrns;
    std::vector<Variable> vars;
    std::stringstream datastring;
    bool is_main_func_present=false;
};

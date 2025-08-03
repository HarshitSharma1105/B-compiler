#include"tokenizer.cpp"
#include<variant>
#include<unordered_map>
#include<unordered_set>
#include<algorithm>


struct AutoVar{
    size_t count; 
};
//TODO : Fix this bs
// struct Arg{
//     std::variant<size_t,int> val;
// };
struct Var{
    size_t offset;
};


struct Literal{
    int literal;
};

struct DataOffset{
    size_t start;
};

typedef std::variant<Var,Literal,DataOffset> Arg;

struct AutoAssign{
    size_t offset;
    Arg arg;
};


struct ExtrnDecl{
    std::string name;
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


struct ScopeBegin{
    std::string name;
};
struct ScopeClose{
    std::string name;
};

struct DataSection{
    std::string concatedstrings;
};

// struct Op{
//     std::variant<AutoVar,AutoAssign,ExtrnDecl,Funcall,FuncDecl,
//     ScopeBegin,ScopeClose> op;
// };

typedef std::variant<AutoVar,AutoAssign,BinOp,ExtrnDecl,Funcall,FuncDecl,
    ScopeBegin,ScopeClose,DataSection> Op;


struct DebugArgVisitor{
    void operator()(const Var& var)
    {
        std::cout << "AutoVar(" << var.offset << ")";
    }

    void operator()(const Literal& literal)
    {
        std::cout << "Literal(" << literal.literal << ")";
    }
    void operator()(const DataOffset& data)
    {
        std::cout << "DataOffset[" << data.start << "]";
    }
};

struct DebugVisitor {
    DebugArgVisitor debugargvisitor{};
    void operator()(const AutoVar& autovar) 
    {
        std::cout << "Auto Variables(" << autovar.count << ")\n";
    }

    void operator()(const AutoAssign& autoassign) 
    {
        std::cout << "Assign AutoVar(" << autoassign.offset << ") = ";
        std::visit(debugargvisitor,autoassign.arg);
        std::cout << "\n";
    }
    void operator()(const BinOp& binop) 
    {
        std::cout << "BinOp (AutoVar(" << binop.index << ")";
        std::cout << ",lhs=";
        std::visit(debugargvisitor,binop.lhs);
        std::cout << ",rhs=";
        std::visit(debugargvisitor,binop.rhs);
        std::cout << ",type=";
        debug(binop.type);
        std::cout << ")\n";
    }
    void operator()(const ExtrnDecl& extrndecl)
    {
        std::cout << "Extrn " << extrndecl.name << "\n";
    }

    void operator()(const Funcall& funcall) 
    {
        std::cout << "Function Call " << funcall.name << "(";
        for (size_t i = 0; i < funcall.args.size(); ++i) {
            std::visit(debugargvisitor,funcall.args[i]);
            if (i != funcall.args.size() - 1) std::cout << ", ";
        }
        std::cout << ")\n";
    }

    void operator()(const FuncDecl& funcdecl) 
    {
        std::cout << "Function Declaration " << funcdecl.name << "(" << funcdecl.count << ")\n";
    }
    void operator()(const ScopeBegin& scope)
    {
        std::cout << "Scope Begin " << scope.name << "\n";
    }
    void operator()(const ScopeClose& scope)
    {
        std::cout << "Scope End " << scope.name  << "\n";
    }

    void operator()(const DataSection& datasection)
    {
        std::cout << "data:\n" << datasection.concatedstrings << "\n";
    }
};


void debug(const std::vector<Op>& ops)
{
    DebugVisitor debugvisitor;
    for(const Op& op:ops)
    {
        std::visit(debugvisitor,op);
    }
}



class IREmittor{
public:
    IREmittor(const std::vector<Token> &tokens) : tokens(tokens){}
    std::vector<Op>   EmitIR()
    {   
        while(peek().has_value())
        {
            if(peek().value().type==Tokentype::funcdecl)
            {
                std::string func_name=consume().val;
                if(func_name=="main")ismainfuncpresent=true;
                try_consume(Tokentype::open_paren,"expcted '('\n");
                while(peek().value().type==Tokentype::identifier)
                {
                    vars[consume().val]=vars_count++;
                    try_consume(Tokentype::comma);
                    //"Expected comma between args\n";
                }
                ops.emplace_back(ScopeBegin{func_name});
                ops.emplace_back(FuncDecl{func_name,vars_count});
                try_consume(Tokentype::close_paren,"expected ')'\n");
                try_consume(Tokentype::open_curly,"expected '{'\n");
                while(true)
                {
                    if(try_peek(Tokentype::extrn))
                    {
                        while(peek().value().type!=Tokentype::semicolon)
                        {
                            std::string extrn_name=consume().val;
                            extrns.insert(extrn_name);
                            ops.emplace_back(ExtrnDecl{extrn_name});
                        }
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(try_consume(Tokentype::auto_).has_value())
                    {
                        size_t curr=vars_count;
                        while(peek().value().type!=Tokentype::semicolon)
                        {
                            try_consume(Tokentype::comma);
                            if(vars.count(peek().value().val))
                            {
                                std::cerr << "variable already declared\n";
                                exit(EXIT_FAILURE);
                            }
                            vars[consume().val]=vars_count++;
                        }
                        ops.emplace_back(AutoVar{vars_count-curr});
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(try_peek(Tokentype::identifier))
                    {
                        if(vars.count(peek().value().val)==0)
                        {
                            std::cerr << "variable not declared " << peek().value().val << "\n";
                            exit(EXIT_FAILURE);
                        }
                        size_t offset = vars[consume().val];
                        try_consume(Tokentype::assignment,"expteced =\n");
                        size_t temp=vars_count++;
                        ops.emplace_back(AutoAssign{offset,compile_expression(0,temp).value()});
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(try_peek(Tokentype::funcall))
                    {
                        std::string funcall_name=consume().val;
                        try_consume(Tokentype::open_paren,"expected '('\n");
                        std::vector<Arg> args;
                        size_t temp=vars_count++;
                        while(try_peek(close_paren)==false)
                        {   
                            args.push_back(compile_expression(0,temp).value());
                            try_consume(Tokentype::comma);
                        }
                        try_consume(Tokentype::close_paren,"expected ')'\n");
                        ops.emplace_back(Funcall{funcall_name,args});
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(try_consume(Tokentype::close_curly))
                    {
                        ops.emplace_back(ScopeClose{func_name});
                        vars_count=0;
                        vars.clear();
                        break;
                    }
                }
            } 
        }
        if(ismainfuncpresent!=true){
            std::cerr << "Main function not declared\n";
            exit(EXIT_FAILURE);
        }
        ops.emplace_back(DataSection{datastring.str()});
        return ops;
    }



private:
    std::optional<Arg> compile_expression(int precedence,size_t temp_index)
    {
        if(precedence==2)return compile_primary_expression();
        std::optional<Arg> lhs=compile_expression(precedence+1,temp_index),rhs;
        while(try_peek(getops(precedence)))
        {
            Tokentype type=consume().type;
            rhs=compile_expression(precedence+1,temp_index);
            ops.emplace_back(BinOp{temp_index,lhs.value(),rhs.value(),type});
            lhs=Var{temp_index};
        }
        return lhs;
    }
    std::optional<Arg> compile_primary_expression()
    {
        switch(peek().value().type)
        {
            case integer_lit:return Literal{atoi(consume().val.c_str())};break;
            case identifier:return Var{(vars[consume().val])};break;
            case string_lit:{
                std::string lit=consume().val;
                for(size_t i=0;i<lit.size();i++)
                {
                    if(lit[i]!='\\')datastring << int(lit[i]);
                    else
                    {
                        i++;
                        if(lit[i]=='n')datastring << 10;
                        else if(lit[i]=='t')datastring << 9;
                        else assert(false && "meesed up escape characters\n");
                        i++;
                    }
                    datastring << ",";
                }
                datastring << "0\n";
                return DataOffset{data_offset++};
        }
            default : assert(false && "TODO : Expressions"); 
        }
        return {};
    }

    std::vector<Tokentype> getops(int precedence)
    {
        switch(precedence)
        {
            case 0:return {Tokentype::add,Tokentype::sub};
            case 1:return {Tokentype::mult,Tokentype::divi};
            default: assert(false && "TODO More binops");
        }
    }
    std::optional<Token> peek(int offset=0){
        if(token_index+offset>=tokens.size()){
            return {};
        }
        return tokens[token_index+offset];
    }
    Token consume(){
        return tokens[token_index++];
    }
    Token try_consume(const Tokentype& type, const std::string& err_msg)
    {
        if (peek().value().type == type) {
            return consume();
        }
        std::cerr << err_msg << std::endl;
        exit(EXIT_FAILURE);
    }
    std::optional<Token> try_consume(const Tokentype& type)
    {
        if (peek().value().type == type) {
            return consume();
        }
        return {};
    }
    bool try_peek(const std::vector<Tokentype>& types)
    {
        return std::any_of(types.begin(),types.end(),[&](Tokentype type){return peek().value().type==type;});
    }
    bool try_peek(const Tokentype& type)
    {
        return peek().value().type == type;
    }
    std::vector<Op> ops;
    std::vector<Token> tokens;
    int token_index=0;
    size_t data_offset=0;
    size_t vars_count=0;
    std::unordered_set<std::string> extrns;
    std::unordered_map<std::string,size_t> vars;
    std::stringstream datastring;
    bool ismainfuncpresent=false;
};
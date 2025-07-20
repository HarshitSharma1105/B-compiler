#include"tokenizer.cpp"
#include<variant>
#include<unordered_map>
#include<unordered_set>


struct AutoVar{
    size_t count; 
};
//TODO : Fix this bs
// struct Arg{
//     std::variant<size_t,int> val;
// };
typedef std::variant<size_t,int> Arg;

struct AutoAssign{
    size_t offset;
    Arg arg;
};


struct ExtrnDecl{
    std::string name;
};

struct AutoPlus{
    size_t index;
    Arg lhs,rhs;
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
struct ScopeClose{};

// struct Op{
//     std::variant<AutoVar,AutoAssign,ExtrnDecl,Funcall,FuncDecl,
//     ScopeBegin,ScopeClose> op;
// };

typedef std::variant<AutoVar,AutoAssign,AutoPlus,ExtrnDecl,Funcall,FuncDecl,
    ScopeBegin,ScopeClose> Op;




struct DebugVisitor {
        struct DebugArgVisitor{
            void operator()(size_t offset)
            {
                std::cout << "AutoVar(" << offset << ")";
            }

            void operator()(int literal)
            {
                std::cout << "Literal(" << literal << ")";
            }
        };
    void operator()(const AutoVar& autovar) 
    {
        std::cout << "Auto Variables (" << autovar.count << ")\n";
    }

    void operator()(const AutoAssign& autoassign) 
    {
        std::cout << "Assign Var(" << autoassign.offset << ") = ";
        std::visit(DebugArgVisitor{},autoassign.arg);
        std::cout << "\n";
    }
    void operator()(const AutoPlus& autoplus) 
    {
        std::cout << "Auto Plus (index=" << autoplus.index << ",";
        std::cout << " lhs=";
        std::visit(DebugArgVisitor{},autoplus.lhs);
        std::cout << " ,rhs=";
        std::visit(DebugArgVisitor{},autoplus.rhs);
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
            std::visit(DebugArgVisitor{},funcall.args[i]);
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
        std::cout << "Scope End\n";
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
                    vars[consume().val]=count++;
                    try_consume(Tokentype::comma);
                    //"Expected comma between args\n";
                }
                ops.emplace_back(ScopeBegin{func_name});
                ops.emplace_back(FuncDecl{func_name,count});
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
                        size_t curr=count;
                        while(peek().value().type!=Tokentype::semicolon)
                        {
                            try_consume(Tokentype::comma);
                            if(vars.count(peek().value().val))
                            {
                                std::cerr << "variable already declared\n";
                                exit(EXIT_FAILURE);
                            }
                            vars[consume().val]=count++;
                        }
                        ops.emplace_back(AutoVar{count-curr});
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
                        ops.emplace_back(AutoAssign{offset,compile_expression().value()});
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(try_peek(Tokentype::funcall))
                    {
                        std::string funcall_name=consume().val;
                        try_consume(Tokentype::open_paren,"expected '('\n");
                        std::vector<Arg> args;
                        while(try_peek(close_paren)==false)
                        {   
                            args.push_back(compile_expression().value());
                            try_consume(Tokentype::comma);
                        }
                        try_consume(Tokentype::close_paren,"expected ')'\n");
                        ops.emplace_back(Funcall{funcall_name,args});
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(try_consume(Tokentype::close_curly))
                    {
                        ops.emplace_back(ScopeClose{});
                        count=0;
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
        return ops;
    }



private:
    std::optional<Arg> compile_expression()
    {
        std::optional<Arg> lhs=compile_primary_expression(),rhs;
        if(try_consume(Tokentype::plus).has_value()==false)return lhs;
        rhs=compile_primary_expression();
        ops.emplace_back(AutoVar{1});
        char temp_name[10];
        size_t index=count;
        sprintf(temp_name,"__temp__%ld",count);
        vars[std::string(temp_name)]=count++;
        //std::cerr << temp_name << std::endl;
        //debug(ops);
        //assert(false && "Do something about Binops");//TODO!!!
        ops.emplace_back(AutoPlus{index,lhs.value(),rhs.value()});
        while(try_consume(Tokentype::plus).has_value())
        {
            lhs=Arg{index};
            rhs=compile_primary_expression();
            ops.emplace_back(AutoPlus{index,lhs.value(),rhs.value()});
        }
        return Arg{index};
    }
    std::optional<Arg> compile_primary_expression()
    {
        switch(peek().value().type)
        {
            case integer_lit:return (atoi(consume().val.c_str()));break;
            case identifier:return Arg{(vars[consume().val])};break;
            case string_lit: assert(false && "TODO:String Literals");
            default : assert(false && "TODO : Expressions"); 
        }
        return {};
    }
    std::optional<Token> peek(int offset=0){
        if(index+offset>=tokens.size()){
            return {};
        }
        return tokens[index+offset];
    }
    Token consume(){
        return tokens[index++];
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
    bool try_peek(const Tokentype& type)
    {
        if (peek().value().type == type) {
            return true;
        }
        return false;
    }
    std::vector<Op> ops;
    std::vector<Token> tokens;
    int index=0;
    size_t count=0;
    std::unordered_set<std::string> extrns;
    std::unordered_map<std::string,size_t> vars;
    bool ismainfuncpresent=false;
};
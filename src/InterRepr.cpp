#include"tokenizer.cpp"
#include<variant>
#include<unordered_map>
#include<unordered_set>


struct AutoVar{
    uint32_t count; 
};
//TODO : Fix this bs
// struct Arg{
//     std::variant<uint32_t,int> val;
// };
typedef std::variant<uint32_t,int> Arg;

struct AutoAssign{
    uint32_t offset;
    Arg arg;
};


struct ExtrnDecl{
    std::string name;
};

struct Funcall{
    std::string name;
    std::vector<Arg> args;
};


struct FuncDecl{
    std::string name;
    uint32_t count;
};


struct ScopeBegin{
    std::string name;
};
struct ScopeClose{};

// struct Op{
//     std::variant<AutoVar,AutoAssign,ExtrnDecl,Funcall,FuncDecl,
//     ScopeBegin,ScopeClose> op;
// };

typedef std::variant<AutoVar,AutoAssign,ExtrnDecl,Funcall,FuncDecl,
    ScopeBegin,ScopeClose> Op;


void print_arg(const Arg& arg) {
    std::visit([](auto&& value) {
        std::cout << value;
    }, arg);
}

struct DebugVisitor {
    void operator()(const AutoVar& autovar) 
    {
        std::cout << "Auto variables (" << autovar.count << ")\n";
    }

    void operator()(const AutoAssign& autoassign) 
    {
        std::cout << "Assign Var(" << autoassign.offset << ")";
        print_arg(autoassign.arg);
        std::cout << "\n";
    }

    void operator()(const ExtrnDecl& extrndecl)
    {
        std::cout << "Extrn " << extrndecl.name << "\n";
    }

    void operator()(const Funcall& funcall) 
    {
        std::cout << "Function call " << funcall.name << "(";
        for (size_t i = 0; i < funcall.args.size(); ++i) {
            print_arg(funcall.args[i]);
            if (i != funcall.args.size() - 1) std::cout << ", ";
        }
        std::cout << ")\n";
    }

    void operator()(const FuncDecl& funcdecl) 
    {
        std::cout << "Function declaration " << funcdecl.name << "(" << funcdecl.count << ")\n";
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
                std::unordered_map<std::string,uint32_t> vars;
                uint32_t count=0;
                try_consume(Tokentype::open_paren,"expcted '('\n");
                while(peek().value().type==Tokentype::identifier)
                {
                    vars[consume().val]=count++;
                    if(peek().value().type==Tokentype::comma)consume();
                }
                ops.emplace_back(ScopeBegin{func_name});
                ops.emplace_back(FuncDecl{func_name,count});
                try_consume(Tokentype::close_paren,"expected ')'\n");
                try_consume(Tokentype::open_curly,"expected '{'\n");
                while(true)
                {
                    if(peek().value().type==Tokentype::extrn)
                    {

                        while(peek().value().type!=Tokentype::semicolon)
                        {
                            std::string extrn_name=consume().val;
                            extrns.insert(extrn_name);
                            ops.emplace_back(ExtrnDecl{extrn_name});
                        }
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(peek().value().type==Tokentype::auto_)
                    {
                        uint32_t curr=count;
                        consume();//consume auto
                        while(peek().value().type!=Tokentype::semicolon)
                        {
                            if(peek().value().type==Tokentype::comma)consume();
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
                    else if(peek().value().type==Tokentype::identifier)
                    {
                        if(vars.count(peek().value().val)==0)
                        {
                            std::cerr << "variable not declared " << peek().value().val << "\n";
                            exit(EXIT_FAILURE);
                        }
                        uint32_t offset = vars[consume().val];
                        try_consume(Tokentype::assignment,"expteced =\n");
                        switch(peek().value().type)
                        {
                            case integer_lit:ops.emplace_back(AutoAssign{offset,atoi(consume().val.c_str())});break;
                            //case string_lit:ops.emplace_back(AutoAssign{offset,soi(consume().val)});break;
                            // TODO : String Literals
                            case identifier:ops.emplace_back(AutoAssign{offset,vars[consume().val]}); break;
                            default: std::cerr << "TODO:Expressions\n";break;
                        }
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(peek().value().type==Tokentype::funcall)
                    {
                        std::string funcall_name=consume().val;
                        try_consume(Tokentype::open_paren,"expected '('\n");
                        bool a=false;
                        std::vector<Arg> args;
                        while(!a)
                        {   
                            switch(peek().value().type)
                            {
                                case integer_lit:args.emplace_back(atoi(consume().val.c_str()));break;
                                //case string_lit: args.emplace_back(consume().val) ;break; 
                                //TODO: string lits;
                                case identifier:args.emplace_back(vars[consume().val]);break;
                                case comma: consume();break;
                                default:a=true;break;
                            }
                        }
                        try_consume(Tokentype::close_paren,"expected ')'\n");
                        ops.emplace_back(Funcall{funcall_name,args});
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(peek().value().type==Tokentype::close_curly)
                    {
                        ops.emplace_back(ScopeClose{});
                        count=0;
                        consume();
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
    std::vector<Op> ops;
    std::vector<Token> tokens;
    int index=0;
    std::unordered_set<std::string> extrns;
    bool ismainfuncpresent=false;
};
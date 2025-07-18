#include"tokenizer.cpp"
#include<variant>
#include<unordered_map>
#include<unordered_set>


struct AutoVar{
    int count; 
};
//TODO : Fix this bs
struct Arg{
    std::variant<std::string,int> val;
};

struct AutoAssign{
    int offset;
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
    int count;
};

struct Op{
    std::variant<AutoVar,AutoAssign,ExtrnDecl,Funcall,FuncDecl> op;
};


void print_arg(const Arg& arg) {
    std::visit([](auto&& value) {
        std::cout << value;
    }, arg.val);
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
};


void debug(const std::vector<Op>& ops)
{
    DebugVisitor debugvisitor;
    for(const Op& op:ops)
    {
        std::visit(debugvisitor,op.op);
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
                std::unordered_map<std::string,int> vars;
                int count=0;
                try_consume(Tokentype::open_paren,"expcted '('\n");
                while(peek().value().type==Tokentype::identifier)
                {
                    vars[consume().val]=count++;
                    if(peek().value().type==Tokentype::comma)consume();
                }
                try_consume(Tokentype::close_paren,"expected ')'\n");
                try_consume(Tokentype::open_curly,"expected '{'\n");
                ops.emplace_back(Op{FuncDecl{func_name, count}});
                while(true)
                {
                    if(peek().value().type==Tokentype::extrn)
                    {

                        while(peek().value().type!=Tokentype::semicolon)
                        {
                            std::string extrn_name=consume().val;
                            extrns.insert(extrn_name);
                            ops.emplace_back(Op{ExtrnDecl{extrn_name}});
                        }
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(peek().value().type==Tokentype::auto_)
                    {
                        int curr=count;
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
                        ops.emplace_back(Op{AutoVar{count-curr}});
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(peek().value().type==Tokentype::identifier)
                    {
                        if(vars.count(peek().value().val)==0)
                        {
                            std::cerr << "variable not declared " << peek().value().val << "\n";
                            exit(EXIT_FAILURE);
                        }
                        int offset=vars[consume().val];
                        try_consume(Tokentype::assignment,"expteced =\n");
                        switch(peek().value().type)
                        {
                            case integer_lit:
                            case string_lit:ops.emplace_back(Op{AutoAssign{offset,consume().val}});break;
                            case identifier:ops.emplace_back(Op{AutoAssign{offset,vars[consume().val]}}); break;
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
                                case integer_lit:
                                case string_lit: args.push_back({consume().val}) ;break;
                                case identifier:args.push_back({vars[consume().val]});break;
                                case comma: consume();break;
                                default:a=true;break;
                            }
                        }
                        try_consume(Tokentype::close_paren,"expected ')'\n");
                        ops.emplace_back(Op{Funcall{funcall_name,args}});
                        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                    }
                    else if(peek().value().type==Tokentype::close_curly)
                    {
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
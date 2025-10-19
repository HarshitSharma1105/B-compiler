#include"tokenizer.cpp"
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


struct Literal{
   size_t literal;
};

struct DataOffset{
    size_t start;
};

struct FuncResult{
    std::string func_name;
};


typedef std::variant<Var,Literal,DataOffset,FuncResult> Arg;

struct AutoAssign{
    size_t index;
    Arg arg;
};


struct ExtrnDecl{
    std::string name;
};


enum UnOpType{
    Negate,
    Not,
    Deref
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
    Arg addr,val;
};

struct DataSection{
    std::string concatedstrings;
};

struct ReturnValue{
    std::optional<Arg> arg;
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
    Loop
};

struct Scope{
    ScopeType type;
    std::string scope_name;
    size_t vars_count,vars_size;
    std::optional<JmpInfo> info;
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


struct DebugArgVisitor{
    void operator()(const Var& var)
    {
        std::cout << "AutoVar(" << var.index << ")";
    }

    void operator()(const Literal& literal)
    {
        std::cout << "Literal(" << literal.literal << ")";
    }
    void operator()(const DataOffset& data)
    {
        std::cout << "DataOffset[" << data.start << "]";
    }
    void operator()(const FuncResult& funcresult)
    {
        std::cout << "FuncResult("  << funcresult.func_name << ")";
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
        std::cout << "Assign AutoVar(" << autoassign.index << ") = ";
        std::visit(debugargvisitor,autoassign.arg);
        std::cout << "\n";
    }
    void operator()(const UnOp& unop)
    {
        std::cout << "UnOp (AutoVar(" << unop.index << ")";
        std::cout << ",arg=";
        std::visit(debugargvisitor,unop.arg);
        std::cout << ",type=";
        switch(unop.type)
        {
            case Negate:std::cout << "negate";break;
            case Not:   std::cout << "not";break;
            case Deref: std::cout << "derefernce";break;
        }
        std::cout << " )\n";
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

    void operator()(const ReturnValue& retval)
    {
        std::cout << "Return Value (";
        if(retval.arg.has_value())std::visit(debugargvisitor,retval.arg.value());
        std::cout << ")\n";
    }
    void operator()(const JmpIfZero& jz)
    {
        std::cout << "Jump To "  << jz.idx << " If (";
        std::visit(debugargvisitor,jz.arg);
        std::cout << "==0)\n";
    }
    void operator()(const Jmp& jmp)
    {
        std::cout << "Jump To " << jmp.idx << "\n";
    }
    void operator()(const Store& store)
    {
        std::cout << "Store ";
        std::visit(debugargvisitor,store.val);
        std::cout << " at address in ";
        std::visit(debugargvisitor,store.addr);
        std::cout << "\n";  
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

static std::vector<std::vector<Tokentype>> precedences =
{
    {Tokentype::assignment},   // we can put plus equals,minus equals,mult equals,assignment everything here
    {Tokentype::less,Tokentype::greater},
    {Tokentype::add,Tokentype::sub},
    {Tokentype::mult,Tokentype::divi}
};

class IREmittor
{
public:
    IREmittor(const std::vector<Token> &tokens) : tokens(tokens){}
    std::vector<Op>   EmitIR()
    {   
        while(peek().has_value())
        {
            compile_statement();
        }
        if(ismainfuncpresent!=true){
            std::cerr << "Main function not declared\n";
            exit(EXIT_FAILURE);
        }
        ops.emplace_back(DataSection{datastring.str()});
        return ops;
    }



private:
    size_t get_var_index(const std::string& name)
    {
        for(size_t i=0;i<vars.size();i++)
        {
            if(vars[i].var_name==name)return vars[i].index;
        }
        return -1;
    }
    void compile_statement()
    {
        if(try_consume(Tokentype::semicolon).has_value())return;
        else if(compile_funcdecl())return;
        else if(compile_extrn())return;
        else if(autovar_dec())return;
        //else if(compile_varinit())return;
        //else if(compile_funcall())return;
        else if(scope_end())return;
        else if(scope_open())return;
        else if(compile_return())return;
        else if(compile_while_loops())return;
        else if(compile_stmt())return;
        assert(false && "UNREACHEABLE\n");
    }

    bool compile_while_loops()
    {
        if(try_consume(Tokentype::while_).has_value())
        {
            JmpInfo info= {.skip_idx=0,.jmp_idx=ops.size()}; // wanna jump at the checking of condition  instruction
            Scope scope={ScopeType::Loop,"",vars_count,vars.size(),info};
            scopes.push(scope);
            Arg arg=compile_expression(0).value();  
            ops.emplace_back(JmpIfZero{arg,0});
            scopes.top().info.value().skip_idx=ops.size()-1;
            try_consume(Tokentype::open_curly,"expected {\n");
            return true;
        }
        return false;
    }


    bool compile_return()
    {
        if(try_consume(Tokentype::return_).has_value())
        {
            std::optional<Arg> arg=compile_expression(0);
            try_consume(Tokentype::semicolon,"Expected ;\n");
            ops.emplace_back(ReturnValue{arg});
            return true;
        }
        return false;
    }

    bool compile_stmt()
    {
        if(compile_expression(0).has_value())
        {
            try_consume(Tokentype::semicolon,"Expected ;\n");
            return true;
        }
        return false;
    }

    bool scope_open()
    {
        if(try_consume(Tokentype::open_curly).has_value())
        {
            scopes.push(Scope{ScopeType::Local,"",vars_count,vars.size(),{}});
            //ops.emplace_back(ScopeBegin{"",ScopeType::Local});
            return true;
        }
        return false;
    }

    bool scope_end()
    {
        if(try_consume(Tokentype::close_curly).has_value())
        {
            Scope scope=scopes.top();
            std::string name=scope.scope_name;
            vars_count=scope.vars_count;
            scopes.pop();
            if(scope.type==ScopeType::Loop)
            {
                JmpInfo info=scope.info.value();
                ops.emplace_back(Jmp{info.jmp_idx});
                std::get<JmpIfZero>(ops[info.skip_idx]).idx=ops.size();
            }
            if(scope.type==ScopeType::Function)ops.emplace_back(ScopeClose{name,scope.type});
            vars.resize(scope.vars_size);
            return true;
        }
        return false;
    }


    // bool compile_funcall()
    // {
    //     if(try_peek(Tokentype::funcall).has_value())
    //     {
    //         std::string funcall_name=consume().val;
    //         try_consume(Tokentype::open_paren,"expected '('\n");
    //         std::vector<Arg> args;
    //         while(try_peek(close_paren).has_value()==false)
    //         {   
    //             args.push_back(compile_expression(0).value());
    //             try_consume(Tokentype::comma);
    //         }
    //         try_consume(Tokentype::close_paren,"expected ')'\n");
    //         ops.emplace_back(Funcall{funcall_name,args});
    //         try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
    //         return true;
    //     }
    //     return false;
    // }
    
    // bool compile_varinit()
    // {
    //     if(try_peek(Tokentype::identifier).has_value())
    //     {
    //         //if(compile_expression(0).has_value())return true; TODO : How to ignore a statement!
    //         if(get_var_index(peek().value().val)==-1)
    //         {
    //             std::cerr << "variable not declared " << peek().value().val << "\n";
    //             exit(EXIT_FAILURE);
    //         }
    //         if(peek(1).value().type!=Tokentype::assignment)return false;
    //         size_t index = get_var_index(consume().val);
    //         consume();// for the assignment operator
    //         ops.emplace_back(AutoAssign{index,compile_expression(0).value()});
    //         try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
    //         return true;
    //     }
    //     return false;
    // }
    bool autovar_dec()
    {
        if(try_consume(Tokentype::auto_).has_value())
        {
            while(peek().value().type!=Tokentype::semicolon)
            {
                try_consume(Tokentype::comma);
                std::string var_name=consume().val;
                if(get_var_index(var_name)!=-1)
                {
                    std::cerr << "variable already declared " << peek().value().val << "\n";
                    exit(EXIT_FAILURE);
                }
                ops.emplace_back(AutoVar{1});
                vars.push_back(Variable{var_name,vars_count++});
                if(try_consume(Tokentype::assignment).has_value())
                {
                    Arg arg=compile_expression(1).value();
                    ops.emplace_back(AutoAssign{get_var_index(var_name),arg});
                }
            }
            try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            return true;
        }
        return false;
    }
    

    bool compile_extrn()
    {
        if(try_peek(Tokentype::extrn).has_value())
        {
            while(peek().value().type!=Tokentype::semicolon)
            {
                std::string extrn_name=consume().val;
                extrns.insert(extrn_name);
                ops.emplace_back(ExtrnDecl{extrn_name});
            }
            try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            return true;
        }
        return false;
    }


    bool compile_funcdecl()
    {
        if(try_peek(Tokentype::funcdecl).has_value())
        {
            std::string func_name=consume().val;
            if(func_name=="main")ismainfuncpresent=true;
            try_consume(Tokentype::open_paren,"expcted '('\n");
            scopes.push(Scope{ScopeType::Function,func_name,vars_count,vars.size(),{}});
            size_t curr=vars_count;
            while(peek().value().type==Tokentype::identifier)
            {
                vars.push_back(Variable{consume().val,vars_count++});
                try_consume(Tokentype::comma);
                //"Expected comma between args\n";
            }
            ops.emplace_back(ScopeBegin{func_name,ScopeType::Function});
            ops.emplace_back(FuncDecl{func_name,vars_count-curr});
            try_consume(Tokentype::close_paren,"expected ')'\n");
            try_consume(Tokentype::open_curly,"expected '{'\n");
            return true;
        }
        return false;
    }
    std::optional<Arg> compile_expression(int precedence)
    {
        if(precedence==precedences.size())return compile_primary_expression();
        std::optional<Arg> lhs=compile_expression(precedence+1),rhs;
        if(try_peek(precedences[precedence])){
            size_t index=vars_count++;
            if(precedence==0)index=std::get<Var>(lhs.value()).index;           
            // TODO : Make this a variant to allow smooth compilation of all statmenets like *p=20,p[1]=20,
            // and to also check at compile time things like 20=3+5 which should be an error as you can't assign to an rvalue
            ops.emplace_back(AutoVar{1});
            Tokentype type=consume().type;
            rhs=compile_expression(precedence+1);
            ops.emplace_back(BinOp{index,lhs.value(),rhs.value(),type});
            lhs=Var{index};
        }
        return lhs;
    }
    std::optional<Arg> compile_primary_expression()
    {
        Token token=consume();
        switch(token.type)
        {
            case Tokentype::identifier:
            {
                Var var=Var{get_var_index(token.val)};
                if(var.index==-1)
                {
                    std::cerr << "variable not declared " << token.val << "\n";
                    exit(EXIT_FAILURE);
                }
                if(try_peek({Tokentype::incr,Tokentype::decr}).has_value()==false)return var;
                Tokentype type=consume().type;
                ops.emplace_back(AutoVar{1});
                ops.emplace_back(AutoAssign{vars_count,var});
                switch(type)
                {
                    case Tokentype::incr:ops.emplace_back(BinOp{var.index,var,Literal{1},Tokentype::add});break;
                    case Tokentype::decr:ops.emplace_back(BinOp{var.index,var,Literal{1},Tokentype::sub});break;
                    default: assert(false && "add more post ops\n");
                }
                return Var{vars_count++};
            }
            case Tokentype::integer_lit:return Literal{(size_t)atoll(token.val.c_str())};
            case Tokentype::string_lit:
            {
                std::string lit=token.val;
                for(size_t i=0;i<lit.size();i++)
                {
                    if(lit[i]!='\\')datastring << int(lit[i]);
                    else
                    {
                        i++;
                        if(lit[i]=='n')datastring << 10;
                        else if(lit[i]=='t')datastring << 9;
                        else assert(false && "messed up escape characters\n");
                        i++;
                    }
                    datastring << ",";
                }
                datastring << "0\n";
                return DataOffset{data_offset++};
            }
            case Tokentype::sub: 
            {
                std::optional<Arg> arg=compile_primary_expression();
                ops.emplace_back(AutoVar{1});
                ops.emplace_back(UnOp{vars_count,arg.value(),UnOpType::Negate});
                return Var{vars_count++};
            }
            case Tokentype::not_:
            {
                std::optional<Arg> arg=compile_primary_expression();
                ops.emplace_back(AutoVar{1});
                ops.emplace_back(UnOp{vars_count,arg.value(),UnOpType::Not});
                return Var{vars_count++};
            }
            case Tokentype::mult:
            {
                std::optional<Arg> arg=compile_primary_expression();
                ops.emplace_back(AutoVar{1});
                if(try_consume(Tokentype::assignment).has_value())
                {
                    Arg val=compile_expression(0).value();
                    ops.emplace_back(Store{arg.value(),val});
                    // TODO : Same as the get<Var> TODO fix this ugly code
                    return val;
                }
                // TODO : something like (*(p+8*i))=val should work
                ops.emplace_back(UnOp{vars_count,arg.value(),UnOpType::Deref});
                return Var{vars_count++};
            }
            case Tokentype::open_paren:
            {
                std::optional<Arg> arg=compile_expression(0);
                try_consume(Tokentype::close_paren,"expected )\n");
                return arg;
            }
            case Tokentype::incr:
            {
                Token var=try_consume(Tokentype::identifier,"expected identifier after pre decrement\n");
                size_t val=get_var_index(var.val);
                if(val==-1)
                {
                    std::cerr << "variable not declared " << token.val << "\n";
                    exit(EXIT_FAILURE);
                }
                ops.emplace_back(BinOp{val,Var{val},Literal{1},Tokentype::add});
                return Var{val};
            }
            case Tokentype::decr:
            {
                Token var=try_consume(Tokentype::identifier,"expected identifier after pre decrement\n");
                size_t val=get_var_index(var.val);
                if(val==-1)
                {
                    std::cerr << "variable not declared " << var.val << "\n";
                    exit(EXIT_FAILURE);
                }
                ops.emplace_back(BinOp{val,Var{val},Literal{1},Tokentype::sub});
                return Var{val};
            }
            case Tokentype::funcall:
            {
                bool ret=try_peek({Tokentype::semicolon,Tokentype::open_curly},-2).has_value();
                // if our tokens are like ;/{  funcall ( then we can assume the result of the func is unused
                //                                     ^ current index since we consumed the funcall token already
                std::string funcall_name=token.val;
                try_consume(Tokentype::open_paren,"expected '('\n");
                std::vector<Arg> args;
                while(try_peek(close_paren).has_value()==false)
                {   
                    args.push_back(compile_expression(0).value());
                    try_consume(Tokentype::comma);
                }
                try_consume(Tokentype::close_paren,"expected ')'\n");
                ops.emplace_back(Funcall{funcall_name,args});
                if(ret)return Var{10011};
                ops.emplace_back(AutoVar{1});
                FuncResult res= {funcall_name};
                ops.emplace_back(AutoAssign{vars_count,res});
                return Var{vars_count++};
            }
            default: debug(ops); assert(false && "UNREACHEABLE\n"); 
        }
        return {};
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
    std::optional<Tokentype> try_peek(const std::vector<Tokentype>& types,int offset=0)
    {
        for(const Tokentype type:types)
        {
            if(peek(offset).value().type==type)return type;
        }
        return {};
    }
    std::optional<Tokentype> try_peek(const Tokentype& type,int offset=0)
    {
        std::vector<Tokentype> t={type};
        return try_peek(t,offset);
    }
    std::vector<Op> ops;
    std::vector<Token> tokens;
    std::stack<Scope> scopes;
    int token_index=0;
    size_t data_offset=0;
    size_t vars_count=0;
    std::unordered_set<std::string> extrns;
    std::vector<Variable> vars;
    std::stringstream datastring;
    bool ismainfuncpresent=false;
};

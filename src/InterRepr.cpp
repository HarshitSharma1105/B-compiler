#include<InterRepr.h>

struct DebugArgVisitor{
    void operator()(const Var& var)
    {
        std::cout << "AutoVar(" << var.index << ")";
    }
    void operator()(const Ref& ref)
    {
        std::cout << "Reference(" << ref.index << ")";
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
        std::visit(debugargvisitor,retval.arg);
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
        std::cout << " at address in AutoVar(" << store.index << ")\n";
    }
};


void debug(const std::vector<Op>& ops)
{
    int i=0;
    DebugVisitor debugvisitor;
    for(const Op& op:ops)
    {
        std::cout << "Operation " << i++ << ": ";
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


IREmittor::IREmittor(const std::vector<Token> &tokens) : tokens(tokens){}
std::vector<Op>   IREmittor::EmitIR()
{   
    while(peek().has_value())
    {
        compile_statement();
    }
    if(is_main_func_present!=true){
        std::cerr << "Main function not declared\n";
        exit(EXIT_FAILURE);
    }
    ops.emplace_back(DataSection{datastring.str()});
    return ops;
}



size_t IREmittor::get_var_index(const std::string& name)
{
    for(size_t i=0;i<vars.size();i++)
    {
        if(vars[i].var_name==name)return vars[i].index;
    }
    return -1;
}
void IREmittor::compile_statement()
{
    if(try_consume(Tokentype::semicolon).has_value())return;
    else if(compile_funcdecl())return;
    else if(compile_extrn())return;
    else if(autovar_dec())return;
    else if(scope_end())return;
    else if(scope_open())return;
    else if(compile_return())return;
    else if(compile_while_loops())return;
    else if(compile_if())return;
    else if(compile_else())return;
    else if(compile_stmt())return;
}


bool IREmittor::compile_if()
{
    if(try_consume(Tokentype::if_).has_value())
    {
        JmpInfo info; // wanna jump at the checking of condition  instruction
        Scope scope={ScopeType::If_,"",vars_count,vars.size(),info};
        scopes.push(scope);
        Arg arg=compile_expression(0);  
        scopes.top().info.skip_idx=ops.size();
        ops.emplace_back(JmpIfZero{arg,0});
        try_consume(Tokentype::open_curly,"expected {\n");
        return true;
    }
    return false;
}


bool IREmittor::compile_else()
{
    if(try_consume(Tokentype::else_).has_value())
    {
        Scope if_scope=scopes.top();
        if(if_scope.type!=ScopeType::If_)
        {
            std::cerr << "Else should be precedded by an if block\n";
            exit(EXIT_FAILURE);
        }
        
        JmpInfo if_info=if_scope.info;
        scopes.pop();

        std::get<JmpIfZero>(ops[if_info.skip_idx]).idx=ops.size();

        Scope scope={ScopeType::Else_,"",vars_count,vars.size(),{.skip_idx=if_info.jmp_idx}};
        scopes.push(scope);

        try_consume(Tokentype::open_curly,"expected {\n");
        return true;
    }
    return false;
}

bool IREmittor::compile_while_loops()
{
    if(try_consume(Tokentype::while_).has_value())
    {
        JmpInfo info= {.skip_idx=0,.jmp_idx=ops.size()}; // wanna jump at the checking of condition  instruction
        Scope scope={ScopeType::Loop,"",vars_count,vars.size(),info};
        scopes.push(scope);
        Arg arg=compile_expression(0);  
        scopes.top().info.skip_idx=ops.size();
        ops.emplace_back(JmpIfZero{arg,0});
        try_consume(Tokentype::open_curly,"expected {\n");
        return true;
    }
    return false;
}


bool IREmittor::compile_return()
{
    if(try_consume(Tokentype::return_).has_value())
    {
        Arg arg=compile_expression(0);
        try_consume(Tokentype::semicolon,"Expected ;\n");
        ops.emplace_back(ReturnValue{arg});
        return true;
    }
    return false;
}

bool IREmittor::compile_stmt()
{
    compile_expression(0);
    try_consume(Tokentype::semicolon,"Expected ;\n");
    return true;
}

bool IREmittor::scope_open()
{
    if(try_consume(Tokentype::open_curly).has_value())
    {
        scopes.push(Scope{ScopeType::Local,"",vars_count,vars.size(),{}});
        //ops.emplace_back(ScopeBegin{"",ScopeType::Local});
        return true;
    }
    return false;
}

bool IREmittor::scope_end()
{
    if(try_consume(Tokentype::close_curly).has_value())
    {
        Scope& scope=scopes.top();
        JmpInfo info=scope.info;
        std::string name=scope.scope_name;
        vars_count=scope.vars_count;
        vars.resize(scope.vars_size);
        if(scope.type==ScopeType::Function)ops.emplace_back(ScopeClose{name,scope.type});
        else if(scope.type==ScopeType::Loop)
        {
            ops.emplace_back(Jmp{info.jmp_idx});
            std::get<JmpIfZero>(ops[info.skip_idx]).idx=ops.size();
        }
        else if(scope.type==ScopeType::If_)
        {
            if(try_peek(Tokentype::else_).has_value())
            {
                scope.info.jmp_idx=ops.size();
                ops.emplace_back(Jmp{0});
                return true;
            }
            std::get<JmpIfZero>(ops[info.skip_idx]).idx=ops.size();
        }
        else if(scope.type==ScopeType::Else_) std::get<Jmp>(ops[info.skip_idx]).idx=ops.size();
        scopes.pop();
        return true;
    }
    return false;
}


bool IREmittor::autovar_dec()
{
    if(try_consume(Tokentype::auto_).has_value())
    {
        while(peek().value().type!=Tokentype::semicolon)
        {
            try_consume(Tokentype::comma);
            std::string var_name=peek().value().val;
            if(get_var_index(var_name)!=-1)
            {
                std::cerr << "variable already declared " << var_name << "\n";
                exit(EXIT_FAILURE);
            }
            ops.emplace_back(AutoVar{1});
            vars.push_back(Variable{var_name,vars_count++});
            compile_expression(0);
        }
        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
        return true;
    }
    return false;
}


bool IREmittor::compile_extrn()
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


bool IREmittor::compile_funcdecl()
{
    if(try_peek(Tokentype::funcdecl).has_value())
    {
        std::string func_name=consume().val;
        if(func_name=="main")is_main_func_present=true;
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
Arg IREmittor::compile_expression(int precedence)
{
    if(precedence==precedences.size())return compile_primary_expression();
    Arg lhs=compile_expression(precedence+1),rhs;
    struct AssignVisitor{
        std::vector<Op>& ops;
        Arg rhs;
        Tokentype type;
        void operator()(const Var &var)
        {
            ops.emplace_back(BinOp{var.index,var,rhs,type});
        }
        void operator()(const Literal& literal)
        {
            std::cerr << "Assignment to integer literals not allowed\n";
            exit(EXIT_FAILURE);
        }
        void operator()(const DataOffset& dataoffset)
        {
            std::cerr << "Assignment to string literals not allowed\n";
            exit(EXIT_FAILURE);
        }
        void operator()(const FuncResult& funcresult)
        {
            std::cerr << "Assignment to function result not allowed\n";
            exit(EXIT_FAILURE);
        }
        void operator()(const Ref& ref)
        {
            ops.emplace_back(Store{ref.index,rhs});
        }
    };
    if(try_peek(precedences[precedence])){
        Tokentype type=consume().type;
        if(precedence==0)
        {
            rhs=compile_expression(0);
            AssignVisitor assignvisitor{ops,rhs,type};
            std::visit(assignvisitor,lhs);
        }          
        else
        {
            rhs=compile_expression(precedence+1);       
            ops.emplace_back(AutoVar{1});
            ops.emplace_back(BinOp{vars_count,lhs,rhs,type});
            lhs=Var{vars_count++};
        }    
    }
    return lhs;
}
Arg IREmittor::compile_primary_expression()
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
            Arg arg=compile_primary_expression();
            ops.emplace_back(AutoVar{1});
            ops.emplace_back(UnOp{vars_count,arg,UnOpType::Negate});
            return Var{vars_count++};
        }
        case Tokentype::not_:
        {
            Arg arg=compile_primary_expression();
            ops.emplace_back(AutoVar{1});
            ops.emplace_back(UnOp{vars_count,arg,UnOpType::Not});
            return Var{vars_count++};
        }
        case Tokentype::mult:
        {
            Arg arg=compile_primary_expression();
            ops.emplace_back(AutoVar{1});
            ops.emplace_back(AutoAssign{vars_count,arg});
            return Ref{vars_count++};
        }
        case Tokentype::open_paren:
        {
            Arg arg=compile_expression(0);
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
            std::string funcall_name=token.val;
            try_consume(Tokentype::open_paren,"expected '('\n");
            std::vector<Arg> args;
            while(try_peek(close_paren).has_value()==false)
            {   
                args.push_back(compile_expression(0));
                try_consume(Tokentype::comma);
            }
            try_consume(Tokentype::close_paren,"expected ')'\n");
            ops.emplace_back(Funcall{funcall_name,args});
            ops.emplace_back(AutoVar{1});
            ops.emplace_back(AutoAssign{vars_count,FuncResult{funcall_name}});
            return Var{vars_count++};
        }
        default: debug(ops); assert(false && "UNREACHEABLE\n"); 
    }
}

std::optional<Token> IREmittor::peek(int offset){
    if(token_index+offset>=tokens.size()){
        return {};
    }
    return tokens[token_index+offset];
} 
Token IREmittor::consume(){
    return tokens[token_index++];
}
Token IREmittor::try_consume(const Tokentype& type, const std::string& err_msg)
{
    if (peek().value().type == type) {
        return consume();
    }
    debug(ops);
    for(int i=0;i<4;i++)
    {
        debug({peek(i).value()});
    }
    std::cerr << err_msg << std::endl;
    exit(EXIT_FAILURE);
}
std::optional<Token> IREmittor::try_consume(const Tokentype& type)
{
    if (peek().value().type == type) {
        return consume();
    }
    return {};
}
std::optional<Tokentype> IREmittor::try_peek(const std::vector<Tokentype>& types,int offset)
{
    for(const Tokentype type:types)
    {
        if(peek(offset).value().type==type)return type;
    }
    return {};
}
std::optional<Tokentype> IREmittor::try_peek(const Tokentype& type,int offset)
{
    std::vector<Tokentype> t={type};
    return try_peek(t,offset);
}


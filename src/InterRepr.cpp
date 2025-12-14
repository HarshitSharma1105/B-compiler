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
            case sub :std::cout << "negate";break;
            case not_:   std::cout << "not";break;
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

    void operator()(const Funcall& funcall) 
    {
        std::cout << "Function Call " << funcall.name << "(";
        for (size_t i = 0; i < funcall.args.size(); ++i) {
            std::visit(debugargvisitor,funcall.args[i]);
            if (i != funcall.args.size() - 1) std::cout << ", ";
        }
        std::cout << ")\n";
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
    void operator()(const Label& label)
    {
        std::cout << "Label(" << label.idx << ")\n";
    }
    void operator()(const JmpIfZero& jz)
    {
        std::cout << "Branch Label("  << jz.idx << ") (";
        std::visit(debugargvisitor,jz.arg);
        std::cout << ")\n";
    }
    void operator()(const Jmp& jmp)
    {
        std::cout << "Branch Label(" << jmp.idx << ")\n";
    }
    void operator()(const Store& store)
    {
        std::cout << "Store ";
        std::visit(debugargvisitor,store.val);
        std::cout << " at AutoVar(" << store.index << ")\n";
    }
    void operator()(const Asm& assembly)
    {
        std::cout << "Assembly Code " << assembly.asm_code << "\n";
    }
};


void debug(const Ops& ops)
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
    {Tokentype::bit_and,Tokentype::bit_or},
    {Tokentype::less,Tokentype::greater,Tokentype::equals,Tokentype::not_equals},
    {Tokentype::shift_left,Tokentype::shift_right},
    {Tokentype::add,Tokentype::sub},
    {Tokentype::mult,Tokentype::divi,Tokentype::remainder}
};


IREmittor::IREmittor(const std::vector<Token> &tokens) : tokens(tokens){}
Compiler   IREmittor::EmitIR()
{   
    while(peek().has_value())
    {
        compile_prog();
    }
    if(is_main_func_present!=true){
        std::cerr << "Main function not declared\n";
        exit(EXIT_FAILURE);
    }
    compiler.data_section=datastring.str();
    compiler.extrns = std::move(extrns);
    return compiler;
}


void IREmittor::compile_prog()
{
    if(try_peek(Tokentype::function))
    {
        Func func{};
        func.function_name=consume().val;
        funcs.insert(func.function_name);
        size_t curr_vars=vars_count;
        size_t vars_size=vars.size();
        if(func.function_name=="main")is_main_func_present=true;
        try_consume(Tokentype::open_paren,"expcted '('\n");
        while(peek().value().type==Tokentype::identifier)
        {
            vars.emplace_back(consume().val,vars_count++);
            try_consume(Tokentype::comma);
            //"Expected comma between args\n";
        }
        func.num_args=vars_count-curr_vars;
        try_consume(Tokentype::close_paren,"expected ')'\n");
        compile_block(func.function_body);
        vars.resize(vars_size);
        max_vars_count=std::max(max_vars_count,vars_count);
        vars_count=curr_vars;
        func.max_vars_count=max_vars_count;
        max_vars_count=0;
        compiler.functions.push_back(func);
    }
    else 
    {
        assert(false && "TODO Global statements");
    }
}


size_t IREmittor::get_var_index(const std::string& name)
{
    for(size_t i=0;i<vars.size();i++)
    {
        if(vars[i].var_name==name)return vars[i].index;
    }
    return -1;
}
void IREmittor::compile_func_body(Ops& ops)
{
    if(try_consume(Tokentype::semicolon))return;
    else if(compile_extrn(ops))return;
    else if(autovar_dec(ops))return;
    else if(compile_scope(ops))return;
    else if(compile_return(ops))return;
    else if(compile_while_loops(ops))return;
    else if(compile_branch(ops))return;
    else if(compile_asm(ops))return;
    compile_stmt(ops);
}




void IREmittor::compile_block(Ops& ops)
{
    if(try_consume(Tokentype::open_curly))
    {
        while(!try_consume(Tokentype::close_curly))compile_func_body(ops);
    }
    else compile_func_body(ops);
}
bool IREmittor::compile_scope(Ops& ops)
{
    if(try_consume(Tokentype::open_curly))
    {
        size_t curr_vars=vars_count;
        size_t vars_size=vars.size();
        while(!try_consume(Tokentype::close_curly))compile_func_body(ops);
        vars.resize(vars_size);
        max_vars_count=std::max(max_vars_count,vars_count);
        vars_count=curr_vars;
        return true;
    }
    return false;
}

bool IREmittor::compile_branch(Ops& ops)
{
    if(try_consume(Tokentype::if_))
    {
        ops.emplace_back(Label{labels_count++});
        size_t curr_vars=vars_count;
        size_t vars_size=vars.size();
        Arg arg=compile_expression(0,ops);  
        size_t jmp=ops.size();
        ops.emplace_back(JmpIfZero{arg,0});
        compile_block(ops);
        size_t if_jump=ops.size();
        ops.emplace_back(Jmp{labels_count});
        ops.emplace_back(Label{labels_count});
        std::get<JmpIfZero>(ops[jmp]).idx=labels_count++;
        vars.resize(vars_size);
        max_vars_count=std::max(max_vars_count,vars_count);
        vars_count=curr_vars;
        if(try_consume(Tokentype::else_))
        {
            curr_vars=vars_count;
            vars_size=vars.size();
            compile_block(ops);
            ops.emplace_back(Label{labels_count});
            std::get<Jmp>(ops[if_jump]).idx=labels_count++;
            vars.resize(vars_size);
            max_vars_count=std::max(max_vars_count,vars_count);
            vars_count=curr_vars;
        }
        return true;
    }
    return false;
}




bool IREmittor::compile_while_loops(Ops& ops)
{
    if(try_consume(Tokentype::while_))
    {
        ops.emplace_back(Label{labels_count});
        size_t start=labels_count++;
        size_t curr_vars=vars_count;
        size_t vars_size=vars.size();
        Arg arg=compile_expression(0,ops);  
        size_t curr=ops.size();
        ops.emplace_back(JmpIfZero{arg,0});
        compile_block(ops);
        ops.emplace_back(Jmp{start});
        ops.emplace_back(Label{labels_count});
        std::get<JmpIfZero>(ops[curr]).idx=labels_count++;
        vars.resize(vars_size);
        max_vars_count=std::max(max_vars_count,vars_count);
        vars_count=curr_vars;
        return true;
    }
    return false;
}


bool IREmittor::compile_return(Ops& ops)
{
    if(try_consume(Tokentype::return_))
    {
        Arg arg=compile_expression(0,ops);
        try_consume(Tokentype::semicolon,"Expected ;\n");
        ops.emplace_back(ReturnValue{arg});
        return true;
    }
    return false;
}




bool IREmittor::autovar_dec(Ops& ops)
{
    if(try_consume(Tokentype::auto_))
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
            vars.emplace_back(var_name,vars_count++);
            compile_expression(0,ops);
        }
        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
        return true;
    }
    return false;
}

bool IREmittor::compile_asm(Ops& ops)
{
    if(try_consume(Tokentype::assembly))
    {
        try_consume(Tokentype::open_paren,"Expected open paren after asm statement");
        std::string code = consume().val;
        ops.emplace_back(Asm{code});
        try_consume(Tokentype::close_paren,"Expected closed paren");
        try_consume(Tokentype::semicolon,"Expected semicolon");
        return true;
    }
    return false;
}
bool IREmittor::compile_extrn(Ops& ops)
{
    if(try_peek(Tokentype::extrn))
    {
        while(peek().value().type!=Tokentype::semicolon)
        {
            std::string extrn_name=consume().val;
            funcs.insert(extrn_name);
            extrns.push_back(extrn_name);
        }
        try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
        return true;
    }
    return false;
}
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

void IREmittor::compile_stmt(Ops& ops)
{
    compile_expression(0,ops);
    try_consume(Tokentype::semicolon,"Expected ;\n");
}


Arg IREmittor::compile_expression(int precedence,Ops& ops)
{
    if(precedence==precedences.size())return compile_primary_expression(ops);
    Arg lhs=compile_expression(precedence+1,ops),rhs;
    while(try_peek(precedences[precedence])){
        Tokentype type=consume().type;
        if(precedence==0)
        {
            rhs=compile_expression(0,ops);
            std::visit(overload{
                [&](const Var& var)  { ops.emplace_back(BinOp{var.index,var,rhs,type});},
                [&](const Ref& ref)  {  ops.emplace_back(Store{ref.index,rhs});},
                [](const auto& ){std::cerr << "Assigning to non assignable value\n"; exit(EXIT_FAILURE); }
            }, lhs);

        }          
        else
        {
            rhs=compile_expression(precedence+1,ops);       
            ops.emplace_back(BinOp{vars_count,lhs,rhs,type});
            lhs=Var{vars_count++};
        }    
    }
    return lhs;
}
Arg IREmittor::compile_primary_expression(Ops& ops)
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
            if(try_peek({Tokentype::incr,Tokentype::decr})==false)return var;
            Tokentype type=consume().type;
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
            Arg arg=compile_primary_expression(ops);
            ops.emplace_back(UnOp{vars_count,arg,token.type});
            return Var{vars_count++};
        }
        case Tokentype::not_:
        {
            Arg arg=compile_primary_expression(ops);
            ops.emplace_back(UnOp{vars_count,arg,token.type});
            return Var{vars_count++};
        }
        case Tokentype::mult:
        {
            Arg arg=compile_primary_expression(ops);
            ops.emplace_back(AutoAssign{vars_count,arg});
            return Ref{vars_count++};
        }
        case Tokentype::open_paren:
        {
            Arg arg=compile_expression(0,ops);
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
        case Tokentype::function:
        {
            std::string funcall_name=token.val;
            try_consume(Tokentype::open_paren,"expected '('\n");
            std::vector<Arg> args;
            while(try_peek(Tokentype::close_paren)==false)
            {   
                args.emplace_back(compile_expression(0,ops));
                try_consume(Tokentype::comma);
            }
            try_consume(Tokentype::close_paren,"expected ')'\n");
            ops.emplace_back(Funcall{funcall_name,args});
            if(funcs.find(funcall_name)==funcs.end())
            {
                std::cerr << "Undeclared function " << funcall_name << "\n";
                exit(EXIT_FAILURE);
            }
            ops.emplace_back(AutoAssign{vars_count,FuncResult{funcall_name}});
            return Var{vars_count++};
        }
        default: debug(ops); debug({token}); assert(false && "UNREACHEABLE\n"); 
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
    std::cerr << err_msg << std::endl;
    exit(EXIT_FAILURE);
}
bool IREmittor::try_consume(const Tokentype& type)
{
    if (peek().value().type == type) {
        consume();
        return true;
    }
    return false;
}
bool IREmittor::try_peek(const std::vector<Tokentype>& types,int offset)
{
    for(const Tokentype type:types)
    {
        if(peek(offset).value().type==type)return true;
    }
    return false;
}
bool IREmittor::try_peek(const Tokentype& type,int offset)
{
    std::vector<Tokentype> t={type};
    return try_peek(t,offset);
}


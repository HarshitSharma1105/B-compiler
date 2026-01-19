#include<InterRepr.h>

struct DebugArgVisitor{
    void operator()(const Var& var)
    {
        switch(var.type)
        {
            case Storage::Auto   :  std::cout << "AutoVar"; break;
            case Storage::Global :  std::cout << "Global"; break;
            case Storage::Array  :  std::cout << "Array"; break;
            default: assert(false && "Unreachable\n");
        }
        std::cout << "(" << var.index << ")";
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
        std::cout << "BinOp (";
        debugargvisitor(binop.var);
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
        std::cout << "Assembly Code " << assembly.asm_code;
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
    return compiler;
}


void IREmittor::compile_prog()
{
    if(Token attrib = try_consume(Tokentype::attribute); attrib || try_peek(Tokentype::function))
    {
        Func func{};
        if(attrib.val=="asm")func.func_flags |= Flag::AsmFunc;
        func.function_name=consume().val;
        size_t vars_size=vars.size();
        if(func.function_name=="main")is_main_func_present=true;
        try_consume(Tokentype::open_paren,"expcted '('\n");
        while(peek().value().type==Tokentype::identifier)
        {
            vars.emplace_back(vars_count++,Storage::Auto,consume().val);
            if(try_consume(Tokentype::assignment))
            {
                Token def_val = try_consume(Tokentype::integer_lit,"Only integer literals are supported as default args for now\n");
                func.default_args.push_back(atoll(def_val.val.c_str()));  
            }
            try_consume(Tokentype::comma);
            //"Expected comma between args\n";
        }
        func.num_args=vars_count;
        compiler.functions.push_back(func);
        Func& back = compiler.functions.back();
        try_consume(Tokentype::close_paren,"expected ')'\n");
        compile_block(back.function_body);
        vars.resize(vars_size);
        max_vars_count=std::max(max_vars_count,vars_count);
        vars_count=0;
        back.max_vars_count=max_vars_count;
        max_vars_count=0;
    }
    else if (try_peek(Tokentype::extrn))
    {
        compile_extrn();
    }
    else if(try_peek(Tokentype::identifier))
    {
        std::string name = consume().val;
        if(try_consume(Tokentype::open_square))
        {
            vars.emplace_back(compiler.arrays.size(),Storage::Array,name);
            size_t val = atoll(try_consume(Tokentype::integer_lit,"Only integer sizes supported for now\n").val.c_str());
            try_consume(Tokentype::close_square,"Expected ]\n");
            compiler.arrays.emplace_back(name,val);
        }
        else 
        {
            vars.emplace_back(compiler.globals.size(),Storage::Global,name);
            compiler.globals.emplace_back(name);
        }
        try_consume(Tokentype::semicolon,"Expected ; after global declaration\n");
    }
    else 
    {
        assert(false && "TODO Global statements");
    }
}


Var IREmittor::get_var(const std::string& name)
{
    for(const auto& var:vars)
    {
        if(var.var_name==name)return var;
    }
    return {(size_t)-1,Storage::Auto,"INVALID"};
}
void IREmittor::compile_func_body(Ops& ops)
{
    if(try_consume(Tokentype::semicolon))return;
    else if(compile_extrn())return;
    else if(autovar_dec(ops))return;
    else if(compile_scope(ops))return;
    else if(compile_return(ops))return;
    else if(compile_while_loops(ops))return;
    else if(compile_for_loops(ops))return;
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


bool IREmittor::compile_for_loops(Ops& ops)
{
    if(try_consume(Tokentype::for_))
    {
        Ops temp{};
        size_t curr_vars=vars_count;
        size_t vars_size=vars.size();
        try_consume(Tokentype::open_paren,"Expected (\n");
        if(!autovar_dec(ops))compile_stmt(ops);
        ops.emplace_back(Label{labels_count});
        size_t start=labels_count++;
        Arg arg=compile_expression(0,ops);
        try_consume(Tokentype::semicolon,"Expected ;\n");  
        size_t curr=ops.size();
        ops.emplace_back(JmpIfZero{arg,0});
        compile_expression(0,temp);
        try_consume(Tokentype::close_paren,"Expected )\n");
        compile_block(ops);
        ops.insert(ops.end(),temp.begin(),temp.end());
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
            if(get_var(var_name).index!=-1)
            {
                std::cerr << "variable already declared " << var_name << "\n";
                exit(EXIT_FAILURE);
            }
            vars.emplace_back(vars_count++,Storage::Auto,var_name);
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
        code.push_back('\n');
        ops.emplace_back(Asm{code});
        try_consume(Tokentype::close_paren,"Expected closed paren");
        try_consume(Tokentype::semicolon,"Expected semicolon");
        return true;
    }
    return false;
}
bool IREmittor::compile_extrn()
{
    if(try_peek(Tokentype::extrn))
    {
        while(peek().value().type!=Tokentype::semicolon)
        {
            std::string extrn_name=consume().val;
            compiler.extrns.push_back(extrn_name);
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
    if(precedence==precedences.size())return compile_prim_expr(ops);
    Arg lhs=compile_expression(precedence+1,ops),rhs;
    while(try_peek(precedences[precedence])){
        Tokentype type=consume().type;
        if(precedence==0)
        {
            rhs=compile_expression(0,ops);
            std::visit(overload{
                [&](const Var& var)  { ops.emplace_back(BinOp{var,var,rhs,type});},
                [&](const Ref& ref)  {  ops.emplace_back(Store{ref.index,rhs});},
                [](const auto& ){std::cerr << "Assigning to non assignable value\n"; exit(EXIT_FAILURE); }
            }, lhs);
        }          
        else
        {
            rhs=compile_expression(precedence+1,ops);       
            ops.emplace_back(BinOp{Var{vars_count,Storage::Auto},lhs,rhs,type});
            lhs=Var{vars_count++,Storage::Auto};
        }    
    }
    return lhs;
}

Arg IREmittor::compile_prim_expr(Ops& ops)
{
    Arg ret = compile_primary_expression(ops);
    Var temp;
    if(try_peek({Tokentype::dot,Tokentype::open_square,Tokentype::incr,Tokentype::decr}))
    {
        while(try_peek({Tokentype::dot,Tokentype::open_square}))
        {
            if(try_consume(Tokentype::dot))
            {
                Arg idx = compile_primary_expression(ops);
                temp = Var{vars_count++,Storage::Auto};
                ops.emplace_back(BinOp{temp,Literal{8},idx,Tokentype::mult});
                ops.emplace_back(BinOp{temp,ret,temp,Tokentype::add});
                ret = Ref{temp.index};
            }
            else if(try_consume(Tokentype::open_square))
            {
                Arg idx = compile_expression(0,ops);
                try_consume(Tokentype::close_square,"Expected closing ']'\n");
                temp = Var{vars_count++,Storage::Auto};
                ops.emplace_back(BinOp{temp,Literal{8},idx,Tokentype::mult});
                ops.emplace_back(BinOp{temp,ret,temp,Tokentype::add});
                ret = Ref{temp.index};
            }
        }
        if(try_peek({Tokentype::incr,Tokentype::decr}))
        {
            if(Var* var = std::get_if<Var>(&ret))
            {
                Tokentype type=consume().type;
                temp = Var{vars_count++,Storage::Auto};
                ops.emplace_back(BinOp{temp,Arg{},ret,Tokentype::assignment});
                switch(type)
                {
                    case Tokentype::incr:ops.emplace_back(BinOp{*var,temp,Literal{1},Tokentype::add});break;
                    case Tokentype::decr:ops.emplace_back(BinOp{*var,temp,Literal{1},Tokentype::sub});break;
                    default: assert(false && "UNREACHABLE\n");
                }
                ret = temp;
            }
            else if(Ref* ref = std::get_if<Ref>(&ret))
            {
                if(try_peek({Tokentype::incr,Tokentype::decr}))
                {
                    size_t curr = ref->index;
                    size_t new_curr = vars_count++;
                    ops.emplace_back(BinOp{Var{new_curr},Arg{},ret,Tokentype::assignment});
                    Tokentype type=consume().type;
                    temp = Var{vars_count++,Storage::Auto};
                    ops.emplace_back(BinOp{temp,Arg{},ret,Tokentype::assignment});
                    switch(type)
                    {
                        case Tokentype::incr:ops.emplace_back(BinOp{temp,temp,Literal{1},Tokentype::add});break;
                        case Tokentype::decr:ops.emplace_back(BinOp{temp,temp,Literal{1},Tokentype::sub});break;
                        default: assert(false && "UNREACHABLE\n");
                    }
                    ops.emplace_back(Store{curr,temp});
                    ret = Var{new_curr};
                }
            }
            else assert(false && "MESSED UP");
        }
    }
    return ret;
}


Arg IREmittor::compile_primary_expression(Ops& ops)
{
    Token token=consume();
    switch(token.type)
    {
        case Tokentype::identifier:
        {
            Var var=get_var(token.val);
            if(var.index==-1)
            {
                std::cerr << "variable not declared " << token.val << "\n";
                exit(EXIT_FAILURE);
            }
            return var;
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
        case Tokentype::not_:
        case Tokentype::bit_not:
        {
            Arg arg=compile_primary_expression(ops);
            ops.emplace_back(UnOp{vars_count,arg,token.type});
            return Var{vars_count++,Storage::Auto};
        }
        case Tokentype::mult:
        {
            Arg arg=compile_primary_expression(ops);
            ops.emplace_back(BinOp{Var{vars_count},Arg{},arg,Tokentype::assignment});
            return Ref{vars_count++};
        }
        case Tokentype::open_paren:
        {
            Arg arg=compile_expression(0,ops);
            try_consume(Tokentype::close_paren,"expected )\n");
            return arg;
        }
        case Tokentype::open_curly:
        {
            std::vector<Arg> args;
            while(try_peek(Tokentype::close_curly)==false)
            {   
                args.emplace_back(compile_expression(0,ops));
                try_consume(Tokentype::comma);
            }
            try_consume(Tokentype::close_curly,"Expected }");
            ops.emplace_back(Funcall{"alloc",{Literal{8*args.size()}}});
            Var ptr  = Var{vars_count++,Storage::Auto};
            Var temp = Var{vars_count++,Storage::Auto};
            ops.emplace_back(BinOp{ptr,Arg{},FuncResult{"alloc"},Tokentype::assignment});
            for(size_t i = 0; i < args.size();i++)
            {
                ops.emplace_back(BinOp{temp,ptr,Literal{8*i},Tokentype::add});
                ops.emplace_back(Store{temp.index,args[i]});
            }
            return ptr;
        }
        case Tokentype::incr:
        {
            Token tok=try_consume(Tokentype::identifier,"expected identifier after pre decrement\n");
            Var var=get_var(tok.val);
            if(var.index==-1)
            {
                std::cerr << "variable not declared " << token.val << "\n";
                exit(EXIT_FAILURE);
            }
            ops.emplace_back(BinOp{Var{var.index},var,Literal{1},Tokentype::add});
            return var;
        }
        case Tokentype::decr:
        {
            Token val=try_consume(Tokentype::identifier,"expected identifier after pre decrement\n");
            Var var=get_var(val.val);
            if(var.index==-1)
            {
                std::cerr << "variable not declared " << val.val << "\n";
                exit(EXIT_FAILURE);
            }
            ops.emplace_back(BinOp{Var{var.index},var,Literal{1},Tokentype::sub});
            return var;
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
            if(
                (
                std::any_of(compiler.functions.begin(),compiler.functions.end(),[funcall_name](const Func& func){return func.function_name==funcall_name;}) 
                || 
                std::any_of(compiler.extrns.begin(),compiler.extrns.end(),[funcall_name](const std::string& extrn){return extrn==funcall_name;})
                )
                == false
            )
            {
                std::cerr << "Undeclared function " << funcall_name << "\n";
                exit(EXIT_FAILURE);
            }
            auto it = std::find_if(compiler.functions.begin(),compiler.functions.end(),[funcall_name](const Func& func){return func.function_name == funcall_name;});
            if(it != compiler.functions.end())
            {
                const Func& func = *it;
                size_t num_args = func.num_args,args_size = args.size(),default_size=func.default_args.size();
                if(num_args>args_size && default_size >= num_args-args_size)
                {
                   for(size_t idx = args_size;idx<num_args;idx++)args.emplace_back(Literal{func.default_args[idx-args_size]});
                }
                else if(num_args!=args_size)
                {
                    for(int i=-10;i<10;i++)debug(std::vector<Token>{peek(i).value()});
                    std::cout << "Function signature for " << funcall_name  << " not matched . Please give default args or provide all non-default arguments to function call\n";
                    std::cout << num_args << ' ' << args_size << ' ' << default_size << '\n';
                    exit(EXIT_FAILURE);
                }
            }
            ops.emplace_back(Funcall{funcall_name,args});
            ops.emplace_back(BinOp{Var{vars_count},Arg{},FuncResult{funcall_name},Tokentype::assignment});
            return Var{vars_count++};
        }
        default: assert(false && "UNREACHEABLE\n"); 
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
Token IREmittor::try_consume(const Tokentype& type)
{
    if (peek().value().type == type) {
        return consume();
    }
    return {Tokentype::invalid};
}
bool IREmittor::try_peek(const std::vector<Tokentype>& types,int offset)
{
    const Tokentype type = peek(offset).value().type;
    for(const Tokentype& expected_type:types)
    {
        if(type==expected_type)return true;
    }
    return false;
}
bool IREmittor::try_peek(const Tokentype& type,int offset)
{
    return try_peek(std::vector<Tokentype>{type},offset);
}


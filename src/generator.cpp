#include"InterRepr.cpp"


class Generator_Mips{
public:
    Generator_Mips(const std::vector<Op> &ops) : ops(ops){}
    std::string generate()
    {
        struct ArgVisitor{
            std::stringstream& stream;
            void operator()(const Var& var)
            {
                stream << "    lw $s0,-" << (var.offset+1)*4 << "($s1)\n";
            }

            void operator()(const Literal& literal)
            {
                stream << "    li $s0," << literal.literal << "\n";
            }
            void operator()(const DataOffset& data)
            {
                stream << "    la $s0,data_" << data.start << "\n";
            }
            void operator()(const FuncResult& funcresult)
            {
                stream << "    move $s0,$a0\n";
            }
        };
        struct Visitor {
            std::stringstream& stream;
            ArgVisitor argvisitor{stream};
            std::string regs[4]={"$a0","$a1","$a2","$a3"};
            void operator()(const AutoVar& autovar) 
            {
                stream << "    addi $sp,$sp,-" << autovar.count*4 << "\n";
            }

            void operator()(const AutoAssign& autoassign) 
            {
                std::visit(argvisitor,autoassign.arg);
                stream << "    sw $s0,-" << (autoassign.offset+1)*4 << "($s1)\n";
            }
            void operator()(const UnOp& unop)
            {
                std::visit(argvisitor,unop.arg);
                stream << "    li $s2,0\n";
                switch(unop.type)
                {
                    case Negate:stream << "    sub $s0,$s2,$s0\n";break;
                    default: assert(false && "TODO More Unary Operations\n");
                } 
                stream << "    sw $s0,-" << (unop.index+1)*4 << "($s1)\n";
            }
            void operator()(const BinOp& binop)
            {
                std::visit(argvisitor,binop.lhs);
                stream << "    move $s2,$s0\n";
                std::visit(argvisitor,binop.rhs);
                switch(binop.type)
                {
                    case Tokentype::add:stream << "    add ";break;
                    case Tokentype::sub:stream << "    sub ";break;
                    case Tokentype::mult:stream << "   mul ";break;
                    default: assert(false && "TODO: MIPS DIVISIO\n");
                }
                stream << " $s0,$s2,$s0\n";
                stream << "    sw $s0,-" << (binop.index+1)*4 << "($s1)\n";
            }

            void operator()(const ExtrnDecl& extrndecl)
            {
                //stream << "    extrn " << extrndecl.name << "\n";
                // nothing to do for extrn symbols for now
            }

            void operator()(const Funcall& funcall) 
            {
                if(funcall.args.size()>4)assert(false && "too many arguments");
                for(size_t i=0;i<funcall.args.size();i++)
                {
                    std::visit(argvisitor,funcall.args[i]);
                    stream << "    move " << regs[i] << ",$s0\n";
                }
                stream << "    jal " << funcall.name << "\n";
            }

            void operator()(const FuncDecl& funcdecl) 
            {
                stream << "    addi $sp,$sp,-" << funcdecl.count*4 << "\n";
                for (int i=0;i<funcdecl.count; i++)
                {
                    stream << "    sw " << regs[i] << ",-" << (i+1)*4 << "($s1)" << "\n";
                }
            }
            void operator()(const ScopeBegin& scope)
            {
                if(scope.type== ScopeType::Function)stream << scope.name << ":\n";
                stream << "    addi $sp,$sp,-8\n";
                stream << "    sw $ra,0($sp)\n";
                stream << "    sw $s1,4($sp)\n";
                stream << "    move $s1,$sp\n";
            }

            void operator()(const ScopeClose& scope)
            {
                stream << "    move $sp,$s1\n";
                stream << "    lw $ra,0($sp)\n";
                stream << "    lw $s1,4($sp)\n";
                stream << "    addi $sp,$sp,8\n";
                if(scope.type== ScopeType::Function)
                {
                    if(scope.name!="main")stream << "    jr $ra\n";
                    else stream << "    li $v0,10\n" << "    syscall\n";
                }
            }
            void operator()(const DataSection& data)
            {
                stream << ".data\n";
                int count=0,idx=0;
                while(idx<data.concatedstrings.size())
                {
                    stream << "data_" << count++ << ": .asciiz ";
                    stream << "\"";
                    while(!(data.concatedstrings[idx]=='0' && data.concatedstrings[idx+1]=='\n'))
                    {
                        int val=0;
                        while(data.concatedstrings[idx]!=',')
                        {
                            val=val*10+(data.concatedstrings[idx++]-'0');
                        }
                        idx++;
                        if(val==10)stream << "\\n";
                        else stream << char(val);
                    }
                    idx+=2;
                    stream << "\"\n";
                }
            }

            void operator()(const ReturnValue& retval)
            {
                std::visit(argvisitor,retval.arg.value());
                stream << "    move $a0,$s0\n";
            }
        };
        textstream << ".text\n";
        textstream << "    .globl main\n";
        generate_stdlib();
        Visitor visitor{textstream};
        while(peek().has_value())
        {
            std::visit(visitor,consume());
        }
        return textstream.str();
    }

private:
    void generate_stdlib()
    {
        textstream << "putchar:\n";
        textstream << "    li $v0,11\n";
        textstream << "    syscall\n";
        textstream << "    jr $ra\n";

        textstream << "putint:\n";
        textstream << "    li $v0,1\n";
        textstream << "    syscall\n";
        textstream << "    jr $ra\n";

        textstream << "puts:\n";
        textstream << "    li $v0,4\n";
        textstream << "    syscall\n";
        textstream << "    jr $ra\n";
    }
    std::optional<Op> peek(int offset=0){
        if(index+offset>=ops.size()){
            return {};
        }
        return ops[index+offset];
    }
    Op consume(){
        return ops[index++];
    }
    std::vector<Op> ops;
    int index=0;
    std::stringstream textstream,datastream;
    std::unordered_set<std::string> extrns;
    bool ismainfuncpresent=false;
};








class Generator_x86_64{
public:
    Generator_x86_64(const std::vector<Op> &ops) : ops(ops){}
    std::string generate()
    {
        struct ArgVisitor{
            std::stringstream& stream;
            void operator()(const Var& var)
            {
                stream << "[rbp-" << (var.offset+1)*8 << "]\n";
            }

            void operator()(const Literal& literal)
            {
                stream << literal.literal << "\n";
            }
            void operator()(const DataOffset& data)
            {
                stream << "data_" << data.start << "\n";
            }
            void operator()(const FuncResult& funcresult)
            {
                stream << "rax\n";
            }
        };
        struct Visitor {
            int count=1;
            std::stringstream& stream;
            ArgVisitor argvisitor{stream};
            std::string regs[4]={"rdi","rsi","rdx","rcx"};
            void operator()(const AutoVar& autovar) 
            {
                count+=autovar.count;
                stream << "    sub rsp," << autovar.count*8 << "\n";
            }

            void operator()(const AutoAssign& autoassign) 
            {
                stream << "    mov rcx,";
                std::visit(argvisitor,autoassign.arg);
                stream << "    mov QWORD [rbp-" << (autoassign.offset+1)*8 << "],rcx\n";
            }
            void operator()(const UnOp& unop)
            {
                stream << "    mov rbx,";
                std::visit(argvisitor,unop.arg);
                stream << "    mov rcx,0\n";
                switch(unop.type)
                {
                    case Negate:stream << "    sub rcx,rbx\n";
                }
                stream << "    mov QWORD [rbp-" << (unop.index+1)*8 << "],rcx\n";
            }
            void operator()(const BinOp& binop)
            {
                stream << "    mov rcx,";
                std::visit(argvisitor,binop.lhs);
                stream << "    mov rbx,";
                std::visit(argvisitor,binop.rhs);
                switch(binop.type)
                {
                    case Tokentype::add:stream << "    add rcx,rbx\n";break;
                    case Tokentype::sub:stream << "    sub rcx,rbx\n";break;
                    case Tokentype::mult:stream << "    imul rcx,rbx\n";break;
                    case Tokentype::divi:stream << "    xor rdx,rdx\n    div rbx\n";assert(false && "MAKE DIVISION TO RCX ALSO\n");
                }
                stream << "    mov QWORD [rbp-" << (binop.index+1)*8 << "],rcx\n";
            }

            void operator()(const ExtrnDecl& extrndecl)
            {
                stream << "    extrn " << extrndecl.name << "\n";
            }

            void operator()(const Funcall& funcall) 
            {
                if(funcall.args.size()>4)assert(false && "too many args");
                for(size_t i=0;i<funcall.args.size();i++)
                {
                    stream << "    mov " << regs[i] << ",";
                    std::visit(argvisitor,funcall.args[i]);
                }
                stream << "    xor rax,rax\n";
                if(count%2)stream << "    sub rsp,8\n";
                stream << "    call " << funcall.name << "\n";
                if(count%2)stream << "    add rsp,8\n";
            }

            void operator()(const FuncDecl& funcdecl) 
            {
                count+=funcdecl.count;
                stream << "    sub rsp," << funcdecl.count*8 << "\n";
                for (int i=0;i<funcdecl.count; i++)
                {
                    stream << "    mov [rbp-" << (i+1)*8 << "]," << regs[i] << "\n";
                }
            }
            void operator()(const ScopeBegin& scope)
            {
                if(scope.type==ScopeType::Function) stream << "public " << scope.name << "\n" << scope.name << ":\n";
                stream << "    push rbp\n";
                stream << "    mov rbp,rsp\n";
            }
            void operator()(const ScopeClose& scope)
            {
                stream << "    mov rsp,rbp\n";
                stream << "    pop rbp\n";
                if(scope.type==ScopeType::Function)stream <<"    ret\n";
            }
            void operator()(const DataSection& data)
            {
                stream << "section \"data\"\n";
                int count=0,idx=0;
                while(idx<data.concatedstrings.size())
                {
                    stream << "data_" << count++ << " db ";
                    while(data.concatedstrings[idx]!='\n')
                    {
                        stream << data.concatedstrings[idx++];
                    }
                    idx++;
                    stream << "\n";
                }
            }
            void operator()(const ReturnValue& retval)
            {
                stream << "    mov rax,";
                std::visit(argvisitor,retval.arg.value());
            }
        };
        textstream << "format ELF64\n";
        textstream << "section \".text\" executable\n";
        Visitor visitor{0,textstream};
        while(peek().has_value())
        {
            std::visit(visitor,consume());
        }
        return textstream.str();
    }



private:
    std::optional<Op> peek(int offset=0){
        if(index+offset>=ops.size()){
            return {};
        }
        return ops[index+offset];
    }
    Op consume(){
        return ops[index++];
    }
    std::vector<Op> ops;
    int index=0;
    std::stringstream textstream;
    std::unordered_set<std::string> extrns;
};

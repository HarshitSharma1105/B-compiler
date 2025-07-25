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
        };
        struct Visitor {
            std::stringstream& stream;
            ArgVisitor argvisitor{stream};
            std::string regs[3]={"$a0","$a1","$a2"};
            void operator()(const AutoVar& autovar) 
            {
                stream << "    addi $sp,$sp,-" << autovar.count*4 << "\n";
            }

            void operator()(const AutoAssign& autoassign) 
            {
                std::visit(argvisitor,autoassign.arg);
                stream << "    sw $s0,-" << (autoassign.offset+1)*4 << "($s1)\n";
            }
            void operator()(const BinOp& binop)
            {
                std::visit(argvisitor,binop.lhs);
                stream << "    move $s2,$s0\n";
                std::visit(argvisitor,binop.rhs);
                switch(binop.type)
                {
                    case BinOpType::add:stream << "    add ";break;
                    case BinOpType::sub:stream << "    sub ";break;
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
                stream << scope.name << ":\n";
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
                if(scope.name!="main")stream << "    jr $ra\n";
                else stream << "    li $v0,10\n" << "    syscall\n";
                // TODO : You dont need to return out of every scope!!
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
        };
        struct Visitor {
            int count=0;
            std::stringstream& stream;
            ArgVisitor argvisitor{stream};
            std::string regs[3]={"rdi","rsi","rdx"};
            void operator()(const AutoVar& autovar) 
            {
                count+=autovar.count;
                stream << "    sub rsp," << autovar.count*8 << "\n";
            }

            void operator()(const AutoAssign& autoassign) 
            {
                stream << "    mov rax,";
                std::visit(argvisitor,autoassign.arg);
                stream << "    mov QWORD [rbp-" << (autoassign.offset+1)*8 << "],rax\n";
            }
            void operator()(const BinOp& binop)
            {
                stream << "    mov rax,";
                std::visit(argvisitor,binop.lhs);
                switch(binop.type)
                {
                    case BinOpType::add:stream << "    add rax,";break;
                    case BinOpType::sub:stream << "    sub rax,";break;
                }
                std::visit(argvisitor,binop.rhs);
                stream << "    mov QWORD [rbp-" << (binop.index+1)*8 << "],rax\n";
            }

            void operator()(const ExtrnDecl& extrndecl)
            {
                stream << "    extrn " << extrndecl.name << "\n";
            }

            void operator()(const Funcall& funcall) 
            {
                
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
                stream << "public " << scope.name << "\n";
                stream << scope.name << ":\n";
                stream << "    push rbp\n";
                stream << "    mov rbp,rsp\n";
            }

            void operator()(const ScopeClose& scope)
            {
                stream << "    mov rsp,rbp\n";
                stream << "    pop rbp\n";
                stream << "    xor rax,rax\n";
                stream << "    ret\n"; 
                // TODO : You dont need to return out of every scope!!
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
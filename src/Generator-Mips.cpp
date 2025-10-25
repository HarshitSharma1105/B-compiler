#include<Generator-Mips.h>



Generator_Mips::Generator_Mips(const std::vector<Op> &ops) : ops(ops){}
std::string Generator_Mips::generate()
{
    struct ArgVisitor{
        std::stringstream& stream;
        void operator()(const Var& var)
        {
            stream << "    lw $s0,-" << (var.index+1)*4 << "($s1)\n";
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
            stream << "    move $s0,$v0\n";
        }
        void operator()(const Ref& ref)
        {
            stream << "    lw $s0,-" << (ref.index+1)*4 << "($s1)\n";
            stream << "    lw $s0,0($s0)\n";
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
            stream << "    sw $s0,-" << (autoassign.index+1)*4 << "($s1)\n";
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
                case Tokentype::assignment:stream << "";break;
                case Tokentype::less:stream       << "    slt $s0,$s2,$s0\n";break;
                case Tokentype::greater:stream    << "    sgt $s0,$s2,$s0\n";break;
                case Tokentype::add:stream        << "    add $s0,$s2,$s0\n";break;
                case Tokentype::sub:stream        << "    sub $s0,$s2,$s0\n";break;
                case Tokentype::mult:stream       << "    mul $s0,$s2,$s0\n";break;
                case Tokentype::divi: assert(false && "TODO MIPS Division\n");
                default: assert(false && "Unknown Binary Operation\n");
            }
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
            stream << "    move $v0,$s0\n";
        }
        void operator()(const JmpIfZero& jz)
        {
            std::visit(argvisitor,jz.arg);
            stream << "    beqz $s0,op_" << jz.idx << "\n";
        }
        void operator()(const Jmp& jmp)
        {
            stream << "    b op_" << jmp.idx << "\n";
        }
        void operator()(const Store& store)
        {
            stream << "    lw $s2,-" << store.index*4 << "($s1)\n";
            std::visit(argvisitor,store.val);
            stream << "    sw $s0,($s2)\n";
        }
    };
    textstream << ".text\n";
    textstream << "    .globl main\n";
    generate_stdlib();
    Visitor visitor{textstream};
    int idx=0;
    while(peek().has_value())
    {
        textstream << "op_" << idx++ << ":\n";
        std::visit(visitor,consume());
    }
    return textstream.str();
}
void Generator_Mips::generate_stdlib()
{
    textstream << "putchar:\n";
    textstream << "    li $v0,11\n";
    textstream << "    syscall\n";
    textstream << "    jr $ra\n";

    textstream << "putint:\n";
    textstream << "    li $v0,1\n";
    textstream << "    syscall\n";
    textstream << "    li $v0,11\n";
    textstream << "    li $a0,32\n";
    textstream << "    syscall\n";
    textstream << "    jr $ra\n";


    textstream << "pnl:\n";
    textstream << "    li $v0,11\n";
    textstream << "    li $a0,10\n";
    textstream << "    syscall\n";
    textstream << "    jr $ra\n";

    

    textstream << "puts:\n";
    textstream << "    li $v0,4\n";
    textstream << "    syscall\n";
    textstream << "    jr $ra\n";


    textstream << "malloc:\n";
    textstream << "    li $v0,9\n";
    textstream << "    syscall\n";
    textstream << "    jr $ra\n";
}
std::optional<Op> Generator_Mips::peek(int offset){
    if(index+offset>=ops.size()){
        return {};
    }
    return ops[index+offset];
}
Op Generator_Mips::consume(){
    return ops[index++];
}
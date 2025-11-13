#include<Generator-Mips.h>

namespace Mips
{
    std::string regs[4]={"$a0","$a1","$a2","$a3"};
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
                stream << "    move " << Mips::regs[i] << ",$s0\n";
            }
            stream << "    jal " << funcall.name << "\n";
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
            std::visit(argvisitor,retval.arg);
            stream << "    move $v0,$s0\n";
            stream << "    move $sp,$s1\n";
            stream << "    lw $ra,0($sp)\n";
            stream << "    lw $s1,4($sp)\n";
            stream << "    addi $sp,$sp,8\n";
            stream << "    jr $ra\n";
        }
        void operator()(const Label& label)
        {
            stream << "label_" << label.idx << ":\n";
        }
        void operator()(const JmpIfZero& jz)
        {
            std::visit(argvisitor,jz.arg);
            stream << "    beqz $s0,label_" << jz.idx << "\n";
        }
        void operator()(const Jmp& jmp)
        {
            stream << "    b label_" << jmp.idx << "\n";
        }
        void operator()(const Store& store)
        {
            stream << "    lw $s2,-" << store.index*4 << "($s1)\n";
            std::visit(argvisitor,store.val);
            stream << "    sw $s0,($s2)\n";
        }
    };
}
Generator_Mips::Generator_Mips(const Compiler& compiler) : compiler(compiler){}
std::string Generator_Mips::generate()
{
    textstream << ".text\n";
    textstream << "    .globl main\n";
    generate_stdlib();
    Mips::Visitor visitor{textstream};
    for(const Func& func:compiler.functions)
    {
        generate_function_prologue(func);
        generate_func(func);
        is_main=(func.function_name=="main");
        generate_function_epilogue();
    }
    std::visit(visitor,Op{DataSection{compiler.data_section}});
    return textstream.str();
}



void Generator_Mips::generate_function_prologue(const Func& func)
{
    textstream << func.function_name << ":\n";
    textstream << "    addi $sp,$sp,-8\n";
    textstream << "    sw $ra,0($sp)\n";
    textstream << "    sw $s1,4($sp)\n";
    textstream << "    move $s1,$sp\n";
    textstream << "    addi $sp,$sp,-" << func.max_vars_count*4 << '\n';
    for (int i=0;i<func.num_args; i++)
    {
        textstream << "    sw " << Mips::regs[i] << ",-" << (i+1)*4 << "($s1)" << "\n";
    }
}
void Generator_Mips::generate_function_epilogue()
{
    textstream << "    move $sp,$s1\n";
    textstream << "    lw $ra,0($sp)\n";
    textstream << "    lw $s1,4($sp)\n";
    textstream << "    addi $sp,$sp,8\n";
    if(!is_main)
    {
        textstream << "    li $a0,0\n";
        textstream << "    jr $ra\n";
    }
    else
    {
        textstream << "    li $v0,10\n syscall\n";
    }
}
void Generator_Mips::generate_func(const Func& func)
{
    Mips::Visitor visitor{textstream};
    for(const Op& op:func.function_body) std::visit(visitor,op);
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

    textstream << "getint:\n";
    textstream << "    li $v0,5\n";
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

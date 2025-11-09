#include<Generator-x86_64.h>


Generator_x86_64::Generator_x86_64(const Compiler& compiler) : compiler(compiler){}
std::string Generator_x86_64::generate()
{
    struct ArgVisitor{
        std::stringstream& stream;
        void operator()(const Var& var)
        {
            stream << "    mov r15,[rbp-" << (var.index+1)*8 << "]\n";
        }

        void operator()(const Literal& literal)
        {
            stream << "    mov r15," << literal.literal << "\n";
        }
        void operator()(const DataOffset& data)
        {
            stream << "    mov r15,data_" << data.start << "\n";
        }
        void operator()(const FuncResult& funcresult)
        {
            stream << "    mov r15,rax\n";
        }
        void operator()(const Ref& ref)
        {
            stream << "    mov r15,[rbp-" << (ref.index+1)*8 << "]\n";
            stream << "    mov r15,[r15]\n";
        }
    };
    struct Visitor {
        std::stringstream& stream;
        std::string regs[6]={"rdi","rsi","rdx","rcx","r8","r9"};
        ArgVisitor argvisitor{stream};
        void operator()(const AutoVar& autovar) 
        {
            stream << "    sub rsp," << autovar.count*8 << "\n";
        }

        void operator()(const AutoAssign& autoassign) 
        {
            std::visit(argvisitor,autoassign.arg);
            stream << "    mov QWORD [rbp-" << (autoassign.index+1)*8 << "],r15\n";
        }
        void operator()(const UnOp& unop)
        {
            std::visit(argvisitor,unop.arg);
            stream << "    mov r14,r15\n";
            switch(unop.type)
            {
                case UnOpType::Negate:stream << "    xor r15,r15\n    sub r15,r14\n";break;
                case UnOpType::Not:   stream << "    cmp r14,0\n    sete al\n    movzx r15,al\n";break;
                default: assert(false && "TODO More Unary Operations\n");
            }
            stream << "    mov QWORD [rbp-" << (unop.index+1)*8 << "],r15\n";
        }
        void operator()(const BinOp& binop)
        {
            std::visit(argvisitor,binop.rhs);
            stream << "    mov r14,r15\n";
            std::visit(argvisitor,binop.lhs);
            switch(binop.type)
            {
                case Tokentype::assignment:stream << "    mov r15,r14\n";break;
                case Tokentype::less:stream       << "    cmp r15,r14\n    setl al\n    movzx r15,al\n";break;
                case Tokentype::greater:stream    << "    cmp r15,r14\n    setg al\n    movzx r15,al\n";break;
                case Tokentype::add:stream        << "    add r15,r14\n";break;
                case Tokentype::sub:stream        << "    sub r15,r14\n";break;
                case Tokentype::mult:stream       << "    imul r15,r14\n";break;
                case Tokentype::divi:stream       << "    xor rdx,rdx\n    div r14\n";assert(false && "MAKE DIVISION TO r15 ALSO\n");
                default: assert(false && "Unknown Binary Operand type\n");
            }
            stream << "    mov QWORD [rbp-" << (binop.index+1)*8 << "],r15\n";
        }

        void operator()(const ExtrnDecl& extrndecl)
        {
            stream << "    extrn " << extrndecl.name << "\n";
        }

        void operator()(const Funcall& funcall) 
        {
            if(funcall.args.size()>6)assert(false && "too many args");
            for(size_t i=0;i<funcall.args.size();i++)
            {
                std::visit(argvisitor,funcall.args[i]);
                stream << "    mov " << regs[i] << ",r15\n";
            }
            stream << "    xor rax,rax\n";
            stream << "    call " << funcall.name << "\n";
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
            std::visit(argvisitor,retval.arg);
            stream << "    mov rax,r15\n";
            stream << "    mov rsp,rbp\n";
            stream << "    pop rbp\n";
            stream << "    ret\n";
        }
        void operator()(const Label& label)
        {
            stream << "label_" << label.idx << ":\n";
        }
        void operator()(const JmpIfZero& jz)
        {
            std::visit(argvisitor,jz.arg);
            stream << "    test r15,r15\n";
            stream << "    jz label_" << jz.idx << "\n";
        }
        void operator()(const Jmp& jmp)
        {
            stream << "    jmp label_" << jmp.idx << "\n";
        }
        void operator()(const Store& store)
        {
            stream << "    mov r14,[rbp-" << store.index*8 << "]\n";
            std::visit(argvisitor,store.val);
            stream << "    mov [r14],r15\n";
        }
    };
    std::string regs[6]={"rdi","rsi","rdx","rcx","r8","r9"};
    textstream << "format ELF64\n";
    textstream << "section \".text\" executable\n";
    Visitor visitor{textstream};
    for(const Func& func:compiler.functions)
    {
        size_t alloc_size=func.max_vars_count;
        if(func.max_vars_count%2)alloc_size++;
        textstream << "public " << func.function_name << "\n" << func.function_name << ":\n";
        textstream << "    push rbp\n";
        textstream << "    mov rbp,rsp\n";
        textstream << "    sub rsp," << 8*alloc_size << "\n";
        for (int i=0;i<func.num_args; i++)
        {
            textstream << "    mov [rbp-" << (i+1)*8 << "]," << regs[i] << "\n";
        }
        for(const Op& op:func.function_body) std::visit(visitor,op);
        textstream << "    mov rsp,rbp\n";
        textstream << "    pop rbp\n";
        textstream << "    mov rax,0\n    ret\n";
    }
    std::visit(visitor,Op{DataSection{compiler.data_section}});
    return textstream.str();
}


#include<Generator-x86_64.h>


void x86_64::ArgVisitor::operator()(const Var& var)
{
    switch(var.type)
    {
        case Storage::Auto:   stream << "    mov r15,[rbp-" << (var.index+1)*8 << "]\n";break;
        case Storage::Global: stream << "    mov r15,[" << var.var_name << "]\n";break;
        case Storage::Array : stream << "    mov r15," <<  var.var_name << "\n";break;
        default: assert(false && "UNREACHABLE\n");
    }
}
void x86_64::ArgVisitor::operator()(const Literal& literal)
{
    stream << "    mov r15," << literal.literal << "\n";
}
void x86_64::ArgVisitor::operator()(const DataOffset& data)
{
    stream << "    mov r15,data_" << data.start << "\n";
}
void x86_64::ArgVisitor::operator()(const FuncResult& funcresult)
{
    stream << "    mov r15,rax\n";
}
void x86_64::ArgVisitor::operator()(const Ref& ref)
{
    stream << "    mov r15,[rbp-" << (ref.index+1)*8 << "]\n";
    stream << "    mov r15,[r15]\n";
}

void x86_64::Visitor::operator()(const UnOp& unop)
{
    std::visit(argvisitor,unop.arg);
    stream << "    mov r14,r15\n";
    switch(unop.type)
    {
        case Tokentype::sub:    stream << "    xor r15,r15\n    sub r15,r14\n";break;
        case Tokentype::not_:   stream << "    cmp r14,0\n    sete al\n    movzx r15,al\n";break;
        case Tokentype::bit_not:stream << "    mov r15,r14\n    not r15\n";break;
        default: assert(false && "Unknown Unary Operation\n");
    }
    stream << "    mov QWORD [rbp-" << (unop.index+1)*8 << "],r15\n";
}
void x86_64::Visitor::operator()(const BinOp& binop)
{
    std::visit(argvisitor,binop.rhs);
    stream << "    mov r14,r15\n";
    std::visit(argvisitor,binop.lhs);
    switch(binop.type)
    {
        case Tokentype::assignment: stream    << "    mov r15,r14\n";break;
        case Tokentype::bit_and:    stream    << "    and r15,r14\n";break;
        case Tokentype::bit_or:     stream    << "    or  r15,r14\n";break;
        case Tokentype::less:       stream    << "    cmp r15,r14\n    setl al\n    movzx r15,al\n";break;
        case Tokentype::greater:    stream    << "    cmp r15,r14\n    setg al\n    movzx r15,al\n";break;
        case Tokentype::equals:     stream    << "    cmp r15,r14\n    sete al\n    movzx r15,al\n";break;
        case Tokentype::not_equals: stream    << "    cmp r15,r14\n    setne al\n   movzx r15,al\n";break;
        case Tokentype::shift_left: stream    << "    mov r11,rcx\n    mov rcx,r14\n    shl r15,cl\n    mov rcx,r11\n";break;
        case Tokentype::shift_right:stream    << "    mov r11,rcx\n    mov rcx,r14\n    shr r15,cl\n    mov rcx,r11\n";break;                
        case Tokentype::add:        stream    << "    add r15,r14\n";break;
        case Tokentype::sub:        stream    << "    sub r15,r14\n";break;
        case Tokentype::mult:       stream    << "    imul r15,r14\n";break;
        case Tokentype::divi:       stream    << "    xor rdx,rdx\n    mov rax,r15\n    idiv r14\n    mov r15,rax\n";break;
        case Tokentype::remainder:  stream    << "    xor rdx,rdx\n    mov rax,r15\n    idiv r14\n    mov r15,rax\n";break;
        default: assert(false && "Unknown Binary Operand type\n");
    }
    switch(binop.var.type)
    {
        case Storage::Auto:   stream << "    mov QWORD [rbp-" << (binop.var.index+1)*8 << "],r15\n";break;
        case Storage::Global: stream << "    mov [" << binop.var.var_name << "],r15\n";break;
        case Storage::Array:  stream << "    mov " << binop.var.var_name << ",r15\n";break;
        default: assert(false && "UNREACHABLE\n");
    }
}
void x86_64::Visitor::operator()(const Funcall& funcall) 
{
    assert(funcall.args.size() <= 6 && "too many args");
    for(size_t i=0;i<funcall.args.size();i++)
    {
        std::visit(argvisitor,funcall.args[i]);
        stream << "    mov " << x86_64::regs[i] << ",r15\n";
    }
    stream << "    xor rax,rax\n";
    stream << "    call " << funcall.name << "\n";
}
void x86_64::Visitor::operator()(const DataSection& data)
{
    stream << "section \"data\" writeable\n";
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
void x86_64::Visitor::operator()(const ReturnValue& retval)
{
    std::visit(argvisitor,retval.arg);
    stream << "    mov rax,r15\n";
    stream << "    mov rsp,rbp\n";
    stream << "    pop rbp\n";
    stream << "    ret\n";
}
void x86_64::Visitor::operator()(const Label& label)
{
    stream << "label_" << label.idx << ":\n";
}
void x86_64::Visitor::operator()(const JmpIfZero& jz)
{
    std::visit(argvisitor,jz.arg);
    stream << "    test r15,r15\n";
    stream << "    jz label_" << jz.idx << "\n";
}
void x86_64::Visitor::operator()(const Jmp& jmp)
{
    stream << "    jmp label_" << jmp.idx << "\n";
}
void x86_64::Visitor::operator()(const Store& store)
{
    stream << "    mov r14,[rbp-" << (store.index+1)*8 << "]\n";
    std::visit(argvisitor,store.val);
    stream << "    mov [r14],r15\n";
}
void x86_64::Visitor::operator()(const Asm& assembly)
{
    stream << assembly.asm_code << '\n';
}



Generator_x86_64::Generator_x86_64(const Compiler& compiler) : compiler(compiler){}
std::string Generator_x86_64::generate()
{
    textstream << "format ELF64\n";
    textstream << "section \"text\" executable\n";
    for(const auto& func:compiler.functions)
    {
        generate_function_prologue(func);
        generate_func(func);
        generate_function_epilogue(func);
    }
    for(const auto& name : compiler.extrns)
    {
        textstream << "    extrn " << name << "\n";
    }
    std::visit(visitor,Op{DataSection{compiler.data_section}});
    for(const auto& name : compiler.globals) 
    {
        textstream << "    public " << name << '\n';
        textstream << name << "  dq 0\n";
    }
    textstream << "section \".bss\"  writeable\n";
    for(const auto& [name,size] : compiler.arrays)
    {
        textstream << "    public " << name << '\n';
        textstream << name << "  rb " << size << '\n';
    }
    return textstream.str();
}

void Generator_x86_64::generate_function_prologue(const Func& func)
{
    assert(func.num_args <= 6 && "too many args");
    size_t alloc_size=func.max_vars_count;
    if(alloc_size%2)alloc_size++;
    textstream << "public " << func.function_name << "\n" << func.function_name << ":\n";
    if((func.func_flags & Flag::AsmFunc) == 0)
    {
        textstream << "    push rbp\n";
        textstream << "    mov rbp,rsp\n";
        textstream << "    sub rsp," << 8*alloc_size << "\n";
        for (int i=0;i<func.num_args; i++)
        {
            textstream << "    mov [rbp-" << (i+1)*8 << "]," << x86_64::regs[i] << "\n";
        }
    }
}
void Generator_x86_64::generate_function_epilogue(const Func& func)
{
    if((func.func_flags & Flag::AsmFunc) == 0)
    {
        textstream << "    mov rsp,rbp\n";
        textstream << "    pop rbp\n";
        textstream << "    mov rax,0\n    ret\n";
    }
}
void Generator_x86_64::generate_func(const Func& func)
{
    for(const auto& op:func.function_body) std::visit(visitor,op);
}
void Generator_x86_64::generate_stdlib()
{
    
}
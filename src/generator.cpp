#include"InterRepr.cpp"



class Generator_Mips{
public:
    Generator_Mips(const std::vector<Op> &ops) : ops(ops){}
    std::string generate()
    {
        textstream << ".text\n";
        textstream << "    .globl main\n";
        datastream << ".data\n";
        generate_stdlib();
        if(ismainfuncpresent!=true){
            std::cerr << "Main function not declared\n";
            exit(EXIT_FAILURE);
        }
        textstream << datastream.str();
        return textstream.str();
    }



private:
    void parse_func(const std::string& func_name,std::vector<std::string> args)
    {
        
    }
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


        textstream << "putstr:\n";
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
            void operator()(size_t offset)
            {
                stream << "    mov rax,[rbp-" << (offset+1)*8 << "]\n";
            }

            void operator()(int literal)
            {
                stream << "    mov rax," << literal << "\n";
            }
        };
        struct Visitor {
            std::stringstream& stream;
            std::string regs[3]={"rdi","rsi","rdx"};
            
            void operator()(const AutoVar& autovar) 
            {
                stream << "    sub rsp," << autovar.count*8 << "\n";
            }

            void operator()(const AutoAssign& autoassign) 
            {
                std::visit(ArgVisitor{stream},autoassign.arg);
                stream << "    mov QWORD [rbp-" << (autoassign.offset+1)*8 << "],rax;\n";
                stream << "\n";
            }
            void operator()(const AutoPlus& autoplus)
            {

            }

            void operator()(const ExtrnDecl& extrndecl)
            {
                stream << "    extrn " << extrndecl.name << "\n";
            }

            void operator()(const Funcall& funcall) 
            {
                
                for(size_t i=0;i<funcall.args.size();i++)
                {
                    std::visit(ArgVisitor{stream},funcall.args[i]);
                    stream << "    mov " << regs[i] << ",rax\n";
                }
                stream << "    call " << funcall.name << "\n";
            }

            void operator()(const FuncDecl& funcdecl) 
            {
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
        };
        textstream << "format ELF64\n";
        textstream << "section \".text\" executable\n";
        Visitor visitor{textstream};
        while(peek().has_value())
        {
            std::visit(visitor,consume());
        }
        return textstream.str();
    }



private:
    void parse_func(const std::string& func_name,std::vector<std::string> args)
    {
        
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
};
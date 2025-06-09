#include"tokenizer.cpp"
#include<unordered_map>
#include<unordered_set>
class Generator{
public:
    Generator(const std::vector<Token> tokens) : tokens(tokens){}
    std::string generate()
    {
        stream << ".text\n";
        stream << ".globl main\n";
        while(peek().type!=Tokentype::endoffile)
        {
            if(peek().type==Tokentype::funcdecl)
            {
                std::string func_name=consume().val;
                stream << func_name << ":\n";
                std::vector<std::string> args;
                try_consume(Tokentype::open_paren,"expcted '('\n");
                while(peek().type==Tokentype::identifier)
                {
                    args.push_back(consume().val);
                    if(peek().type==Tokentype::comma)consume();
                }
                try_consume(Tokentype::close_paren,"expected ')'\n");
                try_consume(Tokentype::open_curly,"expected '{'\n");
                stream << "    addi $sp,$sp,-8\n";
                stream << "    sw $ra,0($sp)\n";
                stream << "    sw $s1,4($sp)\n";
                parse_func(func_name,args);
            }
        }
        generate_stdlib();
        return stream.str();
    }



private:
    void parse_func(const std::string& func_name,std::vector<std::string> args)
    {
        stream << "    move $s1,$sp\n";
        std::string regs[4]={"$a0","$a1","$a2","$a3"};
        if(args.size()>4)std::cerr << "too many arguments\n";
        std::unordered_map<std::string,int> vars;
        int count=0;
        for (int i=0;i<args.size();i++)
        {
            vars[args[i]]=count++;
            stream << "    sd " << regs[i] << "," << -(vars[args[i]]+1)*8 << "($s1)\n";
        }
        while(true)
        {
            if(peek().type==Tokentype::extrn)
            {
                while(peek().type!=Tokentype::semicolon)
                {
                    extrns.insert(consume().val);
                }
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                
            }
            else if (peek().type==Tokentype::auto_)
            {
                consume();
                while(peek().type!=Tokentype::semicolon)
                {
                    if(peek().type==Tokentype::comma)consume();
                    if(vars.count(peek().val))
                    {
                        std::cerr << "variable already declared\n";
                        exit(EXIT_FAILURE);
                    }
                    vars[consume().val]=count++;
                }
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            }
            else if(peek().type==Tokentype::identifier)
            {
                if(vars.count(peek().val)==0)
                {
                    std::cerr << "variable not declared " << peek().val << "\n";
                    exit(EXIT_FAILURE);
                }
                int offset=vars[consume().val];
                try_consume(Tokentype::assignment,"expteced =\n");
                switch(peek().type)
                {
                    case integer_lit: stream << "    li $s0," << peek().val << "\n";break;
                    case identifier: stream << "    ld $s0," << -(vars[peek().val]+1)*8 << "($s1)\n";break;
                    default: std::cout << "fuck off\n";
                }
                consume();//identifier or literal;
                stream << "    sd $s0," << -(offset+1)*8 << "($s1)\n";
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            }
            else if(peek().type==Tokentype::funcall)
            {
                std::string func_name=consume().val;
                try_consume(Tokentype::open_paren,"expected '('\n");
                bool a=0;
                int index=0;
                while(!a)
                {   
                    switch(peek().type)
                    {
                        case integer_lit:stream << "    li " << regs[index++] << ","<< consume().val << "\n";break;
                        case identifier: stream << "    ld " << regs[index++] << "," << -(vars[consume().val]+1)*8 << "($s1)\n";break;
                        case comma: consume();break;
                        default: std::cout << "default assignment\n";a=1;break;
                    }
                }
                try_consume(Tokentype::close_paren,"expected ')'\n");
                stream << "    jal " << func_name << "\n";
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            }
            else if(peek().type==Tokentype::close_curly)
            {
                //stream << "    addi $sp,$sp," << 8*count << "\n";
                stream << "    move $sp,$s1\n";
                stream << "    lw $ra,0($sp)\n";
                stream << "    lw $s1,4($sp)\n";
                stream << "    addi $sp,$sp,8\n";
                if(func_name!="main")stream << "    jr $ra\n";
                else stream << "    li $v0,10\n" << "    syscall\n";
                count=0;
                consume();
                break;
            }
        }
    }
    void generate_stdlib()
    {
        stream << "putchar:\n";
        stream << "    li $v0,11\n";
        stream << "    syscall\n";
        stream << "    jr $ra\n";
    }
    Token peek(int offset=0){
        if(index+offset>=tokens.size()){
            return {Tokentype::endoffile,""};
        }
        return tokens[index+offset];
    }
    Token consume(){
        return tokens[index++];
    }
    Token try_consume(const Tokentype& type, const std::string& err_msg)
    {
        if (peek().type == type) {
            return consume();
        }
        std::cerr << err_msg << std::endl;
        debug({peek(),peek(1),peek(2)});
        exit(EXIT_FAILURE);
    }

    Token try_consume(const Tokentype& type)
    {
        if (peek().type == type) {
            return consume();
        }
        return {Tokentype::endoffile,""};
    }
    std::vector<Token> tokens;
    int index=0;
    std::stringstream stream;
    std::unordered_set<std::string> extrns;
};
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
                try_consume(Tokentype::open_paren,"expcted '('\n");
                char error_msg[100];
                sprintf(error_msg,"expected identifier at function %s",func_name.c_str());
                Token var =try_consume(Tokentype::identifier,error_msg);
                try_consume(Tokentype::close_paren,"expected ')'\n");
                try_consume(Tokentype::open_curly,"expected '{'\n");
                stream << "    addi $sp,$sp,-8\n";
                stream << "    sw $ra,0($sp)\n";
                stream << "    sw $s1,4($sp)\n";
                parse_func(func_name,var);
            }
        }
        generate_stdlib();
        return stream.str();
    }



private:
    void parse_func(const std::string& func_name,Token token)
    {
        stream << "    move $s1,$sp\n";
        std::unordered_map<std::string,int> vars;
        int count=0;
        vars[token.val]=count++;
        stream << "    sd $a0," << -(vars[token.val]+1)*8 << "($s1)\n";
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
                int curr=count;
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
                stream << "    addi $sp,$sp," << (count-curr)*(-8) << "\n";
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
                switch(peek().type)
                {
                    case integer_lit: stream << "    li $a0," << consume().val << "\n";break;
                    case identifier: stream << "    ld $a0," << -(vars[consume().val]+1)*8 << "($s1)\n";break;
                    case close_paren: break;
                    default: std::cout << "default assignment\n";
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
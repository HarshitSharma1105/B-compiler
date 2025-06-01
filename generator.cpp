#include"tokenizer.cpp"
#include<unordered_map>
#include<unordered_set>

class Generator{
public:
    Generator(const std::vector<Token> tokens) : tokens(tokens){}
    std::string generate()
    {
        int count=0;
        stream << ".text\n";
        while(peek().type!=Tokentype::endoffile)
        {
            if(peek().type==Tokentype::funcdecl)
            {
                stream << consume().val << ":\n";
                try_consume(Tokentype::open_paren,"expcted '('\n");
                try_consume(Tokentype::close_paren,"expected ')'\n");
                try_consume(Tokentype::open_curly,"expected '{'\n");
                stream << "    move $s1,$sp\n";
            }
            else if(peek().type==Tokentype::extrn)
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
                stream << "    addi $sp,$sp," << (count-curr)*(-4) << "\n";
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            }
            else if(peek().type==Tokentype::identifier)
            {
                
                if(vars.count(peek().val)==0)
                {
                    std::cerr << "variable not declared\n" << peek().val;
                    exit(EXIT_FAILURE);
                }
                int offset=vars[consume().val];
                try_consume(Tokentype::assignment,"expteced =\n");
                switch(peek().type)
                {
                    case integer_lit: stream << "    li $s0," << peek().val << "\n";break;
                    case identifier: stream << "    lw $s0," << vars[peek().val]*4 << "($s1)\n";break;
                    default: std::cout << "fuck off\n";
                }
                consume();//identifier or literal;
                stream << "    sw $s0," << offset*4 << "($s1)\n";
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            }
            else if(peek().type==Tokentype::funcall)
            {
                
                std::string func_name=consume().val;
                try_consume(Tokentype::open_paren,"expected '('\n");
                switch(peek().type)
                {
                    case integer_lit: stream << "    li $a0," << peek().val << "\n";break;
                    case identifier: stream << "    lw $a0," << vars[peek().val]*4 << "($s1)\n";break;
                    default: std::cout << "fuck offff\n";
                }
                if(peek().type!=Tokentype::close_paren)consume();
                try_consume(Tokentype::close_paren,"expected ')'\n");
                stream << "    jal " << func_name << "\n";
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            }
            else if(peek().type==Tokentype::close_curly)
            {
                stream << "    move $sp,$s1\n";
                stream << "    jr $ra\n";
                count=0;
                consume();
            }
        }
        stream << "    li $v0,10\n" << "    syscall\n";
        generate_stdlib();
        return stream.str();
    }



private:
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
    std::unordered_map<std::string,int> vars;
    std::unordered_set<std::string> extrns;
};
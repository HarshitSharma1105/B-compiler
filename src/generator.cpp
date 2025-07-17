#include"tokenizer.cpp"
#include<unordered_map>
#include<unordered_set>


class Generator{
public:
    Generator(const std::vector<Token> &tokens) : tokens(tokens){}
    std::string generate()
    {
        textstream << ".text\n";
        textstream << "     .globl main\n";
        datastream << ".data\n";
        while(peek().has_value())
        {
            if(peek().value().type==Tokentype::funcdecl)
            {
                std::string func_name=consume().val;
                textstream << func_name << ":\n";
                std::vector<std::string> args;
                try_consume(Tokentype::open_paren,"expcted '('\n");
                while(peek().value().type==Tokentype::identifier)
                {
                    args.push_back(consume().val);
                    if(peek().value().type==Tokentype::comma)consume();
                }
                try_consume(Tokentype::close_paren,"expected ')'\n");
                try_consume(Tokentype::open_curly,"expected '{'\n");
                textstream << "    addi $sp,$sp,-8\n";
                textstream << "    sw $ra,0($sp)\n";
                textstream << "    sw $s1,4($sp)\n";
                parse_func(func_name,args);
            }
        }
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
        if(func_name=="main")ismainfuncpresent=true;
        textstream << "    move $s1,$sp\n";
        if(args.size()!=0)textstream << "    addi $sp,$sp,-" << args.size()*8 << "\n";
        std::string regs[4]={"$a0","$a1","$a2","$a3"};
        if(args.size()>4)std::cerr << "too many arguments\n";
        std::unordered_map<std::string,int> vars;
        int count=args.size();
        for (int i=0;i<args.size();i++)
        {
            vars[args[i]]=i;
            textstream << "    sd " << regs[i] << "," << -(i+1)*8 << "($s1)\n";
        }
        while(true)
        {
            if(peek().value().type==Tokentype::extrn)
            {
                while(peek().value().type!=Tokentype::semicolon)
                {
                    extrns.insert(consume().val);
                }
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
                
            }
            else if (peek().value().type==Tokentype::auto_)
            {
                int curr=count;
                consume();//consume auto
                while(peek().value().type!=Tokentype::semicolon)
                {
                    if(peek().value().type==Tokentype::comma)consume();
                    if(vars.count(peek().value().val))
                    {
                        std::cerr << "variable already declared\n";
                        exit(EXIT_FAILURE);
                    }
                    vars[consume().val]=count++;
                }
                textstream << "    addi $sp,$sp," << (count-curr)*(-8) << "\n";
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            }
            else if(peek().value().type==Tokentype::identifier)
            {
                if(vars.count(peek().value().val)==0)
                {
                    std::cerr << "variable not declared " << peek().value().val << "\n";
                    exit(EXIT_FAILURE);
                }
                int offset=vars[consume().val];
                try_consume(Tokentype::assignment,"expteced =\n");
                switch(peek().value().type)
                {
                    case integer_lit: textstream << "    li $s0," << peek().value().val << "\n";break;
                    case identifier: textstream << "    ld $s0," << -(vars[peek().value().val]+1)*8 << "($s1)\n";break;
                    case string_lit: {
                        datastream << "    data_" << offset << ": .asciiz " << peek().value().val << "\n";
                        textstream << "    la $s0,data_" << offset << "\n";
                    } break;
                    default: std::cout << "TODO:Expressions\n";
                }
                consume();//identifier or literal
                textstream << "    sd $s0," << -(offset+1)*8 << "($s1)\n";
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            }
            else if(peek().value().type==Tokentype::funcall)
            {
                std::string func_name=consume().val;
                try_consume(Tokentype::open_paren,"expected '('\n");
                bool a=false;
                int index=0;
                while(!a)
                {   
                    switch(peek().value().type)
                    {
                        case integer_lit:textstream << "    li " << regs[index++] << ","<< consume().val << "\n";break;
                        case identifier: textstream << "    ld " << regs[index++] << "," << -(vars[consume().val]+1)*8 << "($s1)\n";break;
                        case comma: consume();break;
                        default:a=true;break;
                    }
                }
                try_consume(Tokentype::close_paren,"expected ')'\n");
                textstream << "    jal " << func_name << "\n";
                try_consume(Tokentype::semicolon,"Expected ;\n");//semicolon
            }
            else if(peek().value().type==Tokentype::close_curly)
            {
                textstream << "    move $sp,$s1\n";
                textstream << "    lw $ra,0($sp)\n";
                textstream << "    lw $s1,4($sp)\n";
                textstream << "    addi $sp,$sp,8\n";
                if(func_name!="main")textstream << "    jr $ra\n";
                else textstream << "    li $v0,10\n" << "    syscall\n";
                count=0;
                consume();
                break;
            }
        }
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
    std::optional<Token> peek(int offset=0){
        if(index+offset>=tokens.size()){
            return {};
        }
        return tokens[index+offset];
    }
    Token consume(){
        return tokens[index++];
    }
    Token try_consume(const Tokentype& type, const std::string& err_msg)
    {
        if (peek().value().type == type) {
            return consume();
        }
        std::cerr << err_msg << std::endl;
        exit(EXIT_FAILURE);
    }

    std::optional<Token> try_consume(const Tokentype& type)
    {
        if (peek().value().type == type) {
            return consume();
        }
        return {};
    }
    std::vector<Token> tokens;
    int index=0;
    std::stringstream textstream,datastream;
    std::unordered_set<std::string> extrns;
    bool ismainfuncpresent=false;
};
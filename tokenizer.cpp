#include<vector>
#include<string>
#include<iostream>
#include<sstream>

enum Tokentype{
funcall,
funcdecl,
auto_,
extrn,
identifier,
assignment,
integer_lit,
open_paren,
close_paren,
open_curly,
close_curly,
comma,
semicolon,
endoffile
};

struct Token{
Tokentype type;
std::string val;
};
void debug(const Tokentype& tokentype)
{
    switch (tokentype) {
        case funcall:           std::cout << "function call "; break;
        case funcdecl:          std::cout << "function declaration ";break;
        case auto_:             std::cout << "auto ";break;
        case extrn:             std::cout << "extrn ";break;
        case open_curly:        std::cout << "open_curly "; break;
        case close_curly:       std::cout << "closed_curly "; break;
        case open_paren:        std::cout << "open_paren "; break;
        case close_paren:       std::cout << "close_paren "; break;
        case integer_lit:       std::cout << "integer_lit "; break;
        case assignment:        std::cout << "assignment "; break;
        case identifier:        std::cout << "identifier "; break;
        case comma:             std::cout << "comma "; break;
        case semicolon:         std::cout << "semicolon ";break;
        case endoffile:         std::cout << "end of file ";break;
        default:                std::cout << "Unknown token "; break;
    }
}
void debug(const std::vector<Token> tokens)
{
    for(const Token& token:tokens)
    {
        debug(token.type);
        std::cout << token.val << std::endl;
    }
}


class Tokenizer{
public:
    Tokenizer(const std::string& src): src(src){}
    std::vector<Token> tokenize()
    {
        std::vector<Token> tokens;
        std::string buffer;
        while(true){
            
            if(std::isalpha(peek()))
            {
                buffer.push_back(consume());
                while (std::isalnum(peek()))
                {
                    buffer.push_back(consume());
                }
                if(buffer=="auto")
                {
                    tokens.push_back({Tokentype::auto_,buffer});
                    buffer.clear();
                }
                
                else if(buffer=="extrn")
                {
                    buffer.clear();
                    while(std::isspace(peek()))consume();
                    while(peek()!=';')
                    {
                        if(peek()==',')consume();
                        while(peek()!=',' && peek()!=';')
                        {
                            buffer.push_back(consume());
                        }
                        
                        tokens.push_back({Tokentype::extrn,buffer});
                        buffer.clear();
                    }
                    
                }
                else 
                { 
                    tokens.push_back({Tokentype::identifier,buffer});
                    buffer.clear();
                }
            }
            else if(peek()=='(')
            {
                if(tokens.back().type==Tokentype::identifier)
                {
                    tokens.back().type=Tokentype::funcall;
                }
                int curr=0;
                while(peek(curr++)!=')'){}
                if(peek(curr)=='{')tokens.back().type=Tokentype::funcdecl;
                tokens.push_back({Tokentype::open_paren,"("});
                consume();
            }
            else if(peek()==')')
            {
                tokens.push_back({Tokentype::close_paren,")"});
                consume();
            }
            else if (peek()=='{')
            {
                tokens.push_back({Tokentype::open_curly,"{"});
                consume();
            }
            else if (peek()=='}')
            {
                tokens.push_back({Tokentype::close_curly,"}"});
                consume();
            }
            else if (peek()=='=')
            {
                tokens.push_back({Tokentype::assignment,"="});
                consume();
            }
            
            else if(peek()==',')
            {
                tokens.push_back({Tokentype::comma,","});
                consume();
            }
            else if (std::isdigit(peek())) 
            {
                buffer.push_back(consume());
                while (std::isdigit(peek())) 
                {
                    buffer.push_back(consume());
                }
                tokens.push_back({Tokentype::integer_lit,buffer});
                buffer.clear();
            }
            else if(std::isspace(peek()))
            {
                consume();
            }
            else if(peek()==';')
            {
                consume();
                tokens.push_back({Tokentype::semicolon,";"});
            }
            else if(peek()=='\0')
            {
                consume();
                tokens.push_back({Tokentype::endoffile,""});
                break;
            }
            else
            {
                std::cerr << "errorrred from tokenizer\n";
                exit(EXIT_FAILURE);
            }
        }
        return tokens;
    }
private:
char peek(int offset=0){
    if(index+offset>=src.length()){
        return '@';
    }
    return src[index+offset];
}
char consume(){
    return src[index++];
}
std::string src;
int index=0;
};
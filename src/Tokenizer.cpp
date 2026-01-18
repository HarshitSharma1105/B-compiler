#include<Tokenizer.h>





void debug(const Tokentype& tokentype)
{
    switch (tokentype) {
        case Tokentype::function:          std::cout << "function call "; break;
        case Tokentype::auto_:             std::cout << "auto ";break;
        case Tokentype::extrn:             std::cout << "extrn ";break;
        case Tokentype::open_curly:        std::cout << "open_curly "; break;
        case Tokentype::close_curly:       std::cout << "closed_curly "; break;
        case Tokentype::open_paren:        std::cout << "open_paren "; break;
        case Tokentype::close_paren:       std::cout << "close_paren "; break;
        case Tokentype::open_square:       std::cout << "open_square "; break;
        case Tokentype::close_square:      std::cout << "close_square "; break;
        case Tokentype::integer_lit:       std::cout << "integer_lit "; break;
        case Tokentype::string_lit:        std::cout << "string_lit ";  break;
        case Tokentype::assignment:        std::cout << "assignment "; break;
        case Tokentype::identifier:        std::cout << "identifier "; break;
        case Tokentype::comma:             std::cout << "comma "; break;
        case Tokentype::semicolon:         std::cout << "semicolon ";break;
        case Tokentype::add:               std::cout << "add ";break;
        case Tokentype::sub:               std::cout << "sub ";break;
        case Tokentype::mult:              std::cout << "mult ";break;
        case Tokentype::divi:              std::cout << "divi ";break;
        case Tokentype::remainder:         std::cout << "remainder ";break;
        case Tokentype::incr:              std::cout << "incr ";break;
        case Tokentype::decr:              std::cout << "decr ";break;
        case Tokentype::return_:           std::cout << "return ";break;
        case Tokentype::while_:            std::cout << "while ";break;
        case Tokentype::for_:              std::cout << "for ";break;
        case Tokentype::less:              std::cout << "less ";break;
        case Tokentype::greater:           std::cout << "greater ";break;
        case Tokentype::equals:            std::cout << "equals ";break;
        case Tokentype::not_equals:        std::cout << "not equals ";break; 
        case Tokentype::not_:              std::cout << "negate ";break;
        case Tokentype::if_:               std::cout << "if ";break;
        case Tokentype::else_:             std::cout << "else ";break;
        case Tokentype::shift_left:        std::cout << "shift-left ";break;
        case Tokentype::shift_right:       std::cout << "shift-right ";break;
        case Tokentype::bit_and:           std::cout << "bitwise-and ";break;
        case Tokentype::bit_or:            std::cout << "bitwise-or ";break;
        case Tokentype::bit_not:           std::cout << "bitwise-inverse ";break;
        case Tokentype::assembly:          std::cout << "asm ";break;
        case Tokentype::dot:               std::cout << "dot ";break;
        default:                           std::cout << "Unknown token "; break;
    }
}
void debug(const std::vector<Token>& tokens)
{
    for(const Token& token:tokens)
    {
        debug(token.type);
        std::cout << token.val << std::endl;
    }
}
Token::operator bool()
{
    return type != Tokentype::invalid;
}



Tokenizer::Tokenizer(const std::string& src): src(src){}
std::vector<Token> Tokenizer::tokenize()
{
    std::vector<Token> tokens;
    std::string buffer;
    while(peek().has_value()){            
        if(std::isalpha(peek().value()))
        {
            buffer.push_back(consume());
            while (std::isalnum(peek().value()) || peek().value() == '_')
            {
                buffer.push_back(consume());
            }
            if(buffer=="auto")
            {
                tokens.push_back({Tokentype::auto_});
            }
            else if(buffer=="extrn")
            {
                buffer.clear();
                while(std::isspace(peek().value()))consume();
                while(peek().value()!=';')
                {
                    if(peek().value()==',')consume();
                    while(peek().value()!=',' && peek().value()!=';')
                    {
                        buffer.push_back(consume());
                    }
                    
                    tokens.push_back({Tokentype::extrn,buffer});
                    buffer.clear();
                }
            }
            else if(buffer=="return")
            {
                tokens.push_back({Tokentype::return_});
            }
            else if(buffer=="while")
            {
                tokens.push_back({Tokentype::while_});
            }
            else if (buffer == "for")
            {
                tokens.push_back({Tokentype::for_});
            }
            else if(buffer=="if")
            {
                tokens.push_back({Tokentype::if_});
            }
            else if(buffer=="else")
            {
                tokens.push_back({Tokentype::else_});
            }
            else if(buffer=="asm")
            {
                tokens.push_back({Tokentype::assembly});
            }
            else if(buffer=="true")
            {
                tokens.push_back({Tokentype::integer_lit,"1"});
            }
            else if(buffer=="false" || buffer == "null" || buffer == "NULL")
            {
                tokens.push_back({Tokentype::integer_lit,"0"});
            }
            else 
            { 
                tokens.push_back({Tokentype::identifier,buffer});
            }
            buffer.clear();
        }
        else if(peek().value()=='(')
        {
            if(tokens.back().type==Tokentype::identifier || tokens.back().type==Tokentype::integer_lit)
            {
                tokens.back().type=Tokentype::function;
            }
            tokens.push_back({Tokentype::open_paren,"("});
            consume();   
        }
        else if(peek().value()=='[')
        {
            if(peek(1).value() == '[')
            {
                consume();
                consume();
                buffer.clear();
                while (std::isalnum(peek().value())) buffer.push_back(consume());
                assert(buffer.empty()==false);
                tokens.push_back({Tokentype::attribute,buffer});
                buffer.clear();
                assert(consume()==']');
                assert(consume()==']');
            }
            else
            {
                tokens.push_back({Tokentype::open_square,"["});
                consume();
            }
        }
        else if(peek().value()==']')
        {
            tokens.push_back({Tokentype::close_square,"]"});
            consume();
        }
        else if(peek().value()==')')
        {
            tokens.push_back({Tokentype::close_paren,")"});
            consume();
        }
        else if(peek().value()=='+')
        {
            if(peek(1).value()=='+')
            {
                tokens.push_back({Tokentype::incr,"++"});
                consume();
            }
            else tokens.push_back({Tokentype::add,"+"});
            consume();
        }
        else if(peek().value()=='-')
        {
            if(peek(1).value()=='-')
            {
                tokens.push_back({Tokentype::decr,"--"});
                consume();
            }
            else tokens.push_back({Tokentype::sub,"-"});
            consume();
        }
        else if(peek().value()=='*')
        {
            tokens.push_back({Tokentype::mult,"*"});
            consume();
        }
        else if(peek().value()=='/')
        {
            if(peek(1).value()=='/')
            {
                consume();
                consume();
                while(peek().has_value() && peek().value() != '\n')consume();
                consume();
                continue;
            }
            else if(peek(1).value()=='*')
            {
                consume();
                consume();
                while(!(peek().has_value() && peek().value()=='*' && peek(1).has_value() && peek(1).value()=='/'))consume();
                consume();
                consume();
                continue;
            }
            tokens.push_back({Tokentype::divi,"/"});
            consume();
        }
        else if(peek().value() == '.')
        {
            tokens.push_back({Tokentype::dot,"."});
            consume();
        }
        else if(peek().value()=='"')   
        {
            consume();
            while(true)
            {
                if(peek().value() == '\"')
                {
                    if(buffer.back()=='\\')buffer.pop_back();
                    else break;
                }
                buffer.push_back(consume());
            }
            consume();
            tokens.push_back({Tokentype::string_lit,buffer});
            buffer.clear();
        }
        else if (peek().value()=='{')
        {
            tokens.push_back({Tokentype::open_curly,"{"});
            consume();
        }
        else if (peek().value()=='}')
        {
            tokens.push_back({Tokentype::close_curly,"}"});
            consume();
        }
        else if (peek().value()=='=')
        {
            if(peek(1).value()=='=')
            {
                tokens.push_back({Tokentype::equals,"=="});
                consume();
            }
            else tokens.push_back({Tokentype::assignment,"="});
            consume();
        }
        else if(peek().value()=='!')
        {
            if(peek(1).value()=='=')
            {
                tokens.push_back({Tokentype::not_equals,"=="});
                consume();
            }
            else tokens.push_back({Tokentype::not_,"!"});
            consume();
        }
        else if(peek().value()==',')
        {
            tokens.push_back({Tokentype::comma,","});
            consume();
        }
        else if(peek().value()=='<')
        {
            if(peek(1).value() == '<')
            {
                tokens.push_back({Tokentype::shift_left,"<<"});
                consume();
            }
            else tokens.push_back({Tokentype::less,"<"});  // TODO : Lesser Equals
            consume();
        }
        else if(peek().value()=='>')
        {
            if(peek(1).value() == '>')
            {
                tokens.push_back({Tokentype::shift_right,">>"});
                consume();
            }
            else tokens.push_back({Tokentype::greater,">"}); // TODO : Greater Equals
            consume();
        }
        else if(peek().value() == '%')
        {
            tokens.push_back({Tokentype::remainder,"%"});
            consume();
        }
        else if (peek().value() == '&')
        {
            tokens.push_back({Tokentype::bit_and,"&"});
            consume();
        }
        else if (peek().value() == '|')
        {
            tokens.push_back({Tokentype::bit_or,"|"});
            consume();
        }
        else if (peek().value() == '~')
        {
            tokens.push_back({Tokentype::bit_not,"~"});
            consume();
        }
        else if (peek().value() == '\'')
        {
            consume();
            char ch = consume();
            if(consume()!='\'')
            {
                std::cerr << "Character literals can only be one character long\n";
                exit(EXIT_FAILURE);
            }
            tokens.push_back({Tokentype::integer_lit,std::to_string(int(ch))});
        }
        else if (std::isdigit(peek().value())) 
        {
            buffer.push_back(consume());
            while (std::isdigit(peek().value())) 
            {
                buffer.push_back(consume());
            }
            tokens.push_back({Tokentype::integer_lit,buffer});
            buffer.clear();
        }
        else if(std::isspace(peek().value()))
        {
            consume();
        }
        else if(peek().value()==';')
        {
            consume();
            tokens.push_back({Tokentype::semicolon,";"});
        }
        else
        {
            std::cerr << "Unrecognized Token: " << peek().value() << "\n";
            exit(EXIT_FAILURE);
        }
    }
    return tokens;
}
std::optional<char> Tokenizer::peek(int offset){
    if(index+offset>=src.size())return {};
    return src[index+offset];
}
char Tokenizer::consume(){
    return src[index++];
}


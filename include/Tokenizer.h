#pragma once



#include<Preprocessor.h>
#include<vector>
#include<optional>
#include<assert.h>


enum Tokentype{
funcall,
funcdecl,
auto_,
extrn,
identifier,
assignment,
integer_lit,
string_lit,
open_paren,
close_paren,
open_curly,
close_curly,
comma,
semicolon,
add,
sub,
mult,
divi,
less,
greater,
incr,
decr,
return_,
while_,
not_
};

struct Token{
    Tokentype type;
    std::string val;
};
void debug(const Tokentype& tokentype);
void debug(const std::vector<Token>& tokens);


class Tokenizer{
public:
    Tokenizer(const std::string& src);
    std::vector<Token> tokenize();

private:
    std::optional<char> peek(int offset=0);
    char consume();
    std::string src;
    int index=0;
};

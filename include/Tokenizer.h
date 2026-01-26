#pragma once



#include<Preprocessor.h>
#include<vector>
#include<optional>
#include<assert.h>


enum Tokentype{
function,
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
open_square,
close_square,
comma,
semicolon,
add,
sub,
mult,
divi,
remainder,
less,
greater,
equals,
not_equals,
incr,
decr,
return_,
while_,
for_,
not_,
if_,
else_,
shift_left,
shift_right,
bit_or,
bit_and,
bit_not,
assembly,
dot,
attribute,
switch_,
case_,
default_,
break_,
colon,
decl,
invalid
};

struct Token{
    Tokentype type;
    std::string val;
    operator bool();
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

#pragma once

#include<InterRepr.h>




class Generator_x86_64
{
public:
    Generator_x86_64(const std::vector<Op> &ops);
    std::string generate();

private:
    void generate_stdlib();
    std::optional<Op> peek(int offset=0);
    Op consume();
    std::vector<Op> ops;
    int index=0;
    std::stringstream textstream;
    std::unordered_set<std::string> extrns;
};

#pragma once

#include <Tokenizer.h>

#include <algorithm>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <variant>

using big_int = __int128_t;

// help for debug

inline std::ostream &operator<<(std::ostream &os, __int128 value) {
  if (value == 0)
    return os << '0';

  bool neg = value < 0;
  if (neg)
    value = -value;

  std::string s;
  while (value > 0) {
    s.push_back('0' + value % 10);
    value /= 10;
  }

  if (neg)
    s.push_back('-');
  std::reverse(s.begin(), s.end());

  return os << s;
}

enum Storage { Auto, Global, Array };

struct Var {
  size_t index;
  Storage type;
  std::string var_name;
};

struct Ref {
  size_t index;
};

struct Literal {
  big_int literal;
};

struct DataOffset {
  size_t start;
};

struct FuncResult {
  std::string func_name;
};

struct NoArg {};

typedef std::variant<Var, Ref, Literal, DataOffset, FuncResult, NoArg> Arg;

struct UnOp {
  size_t index;
  Arg arg;
  Tokentype type;
};

struct BinOp {
  Var var;
  Arg lhs, rhs;
  Tokentype type;
};

struct Funcall {
  std::string name;
  std::vector<Arg> args;
  size_t stk = 0;
};

struct Store {
  size_t index;
  Arg val;
};

struct DataSection {
  std::string concatedstrings;
};

struct ReturnValue {
  Arg arg;
};

struct Label {
  size_t idx;
};

struct JmpIfZero {
  Arg arg;
  size_t idx;
};

struct Jmp {
  size_t idx;
};

struct Asm {
  std::string asm_code;
};

typedef std::variant<UnOp, BinOp, Funcall, DataSection, ReturnValue, JmpIfZero,
                     Jmp, Label, Store, Asm>
    Op;

typedef std::vector<Op> Ops;

struct Func {
  Ops function_body;
  std::string function_name;
  size_t max_vars_count, num_args, func_flags;
  std::vector<size_t> default_args;
};

template <class... Ts> struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overload(Ts...) -> overload<Ts...>;

struct Compiler {
  std::vector<Func> functions;
  std::string data_section;
  std::vector<std::string> extrns;
  std::vector<std::pair<std::string, size_t>> arrays, globals;
};
void debug(const Compiler &compiler);

enum Flag { AsmFunc = 1 << 1 };

void debug(const Ops &ops);

class IREmittor {
public:
  IREmittor(const std::vector<Token> &tokens);
  Compiler EmitIR();
  void show_table();

public:
  Var get_var(const std::string &name);
  void compile_prog();
  void compile_func_body(Ops &ops);
  bool compile_while_loops(Ops &ops);
  bool compile_for_loops(Ops &ops);
  bool compile_return(Ops &ops);
  void compile_stmt(Ops &ops);
  bool autovar_dec(Ops &ops);
  bool compile_extrn();
  bool compile_branch(Ops &ops);
  void compile_block(Ops &ops);
  bool compile_scope(Ops &ops);
  bool compile_asm(Ops &ops);
  bool compile_switch(Ops &ops);
  Arg compile_expression(int precedence, Ops &ops);
  Arg compile_prim_expr(Ops &ops);
  Arg compile_primary_expression(Ops &ops);
  Tokentype conv(const Tokentype &type);
  std::optional<Token> peek(int offset = 0);
  Token consume();
  Token try_consume(const Tokentype &type, const std::string &err_msg);
  Token try_consume(const Tokentype &type);
  bool try_peek(const std::vector<Tokentype> &types, int offset = 0);
  bool try_peek(const Tokentype &type, int offset = 0);
  Compiler compiler{};
  std::vector<Token> tokens;
  int token_index = 0;
  size_t data_offset = 0;
  size_t vars_count = 0;
  size_t max_vars_count = 0;
  size_t labels_count = 0;
  std::vector<Var> vars;
  std::stringstream datastring;
  std::unordered_set<std::string> functions;
  bool is_main_func_present = false;

private:
  std::unordered_map<size_t, big_int> const_vars;
  std::optional<big_int> get_const(const Arg &);
  void set_const(const Var &, big_int);
  void remove(const Var &);
};

// helper function
big_int eval_binop(big_int lhs, big_int rhs, Tokentype type);
big_int eval_unop(big_int, Tokentype);

#pragma once

#include <InterRepr.h>

namespace arm64_apple_darwin {
inline std::array<std::string, 8> regs = {"x1", "x2", "x3", "x4",
                                          "x5", "x6", "x7", "x8"};
struct ArgVisitor {
  std::stringstream &stream;
  void operator()(const Var &var);
  void operator()(const Literal &literal);
  void operator()(const DataOffset &data);
  void operator()(const FuncResult &funcresult);
  void operator()(const Ref &ref);
  void operator()(const NoArg &noarg);
};
struct Visitor {
  std::stringstream &stream;
  ArgVisitor argvisitor{stream};
  void operator()(const UnOp &unop);
  void operator()(const BinOp &binop);
  void operator()(const DataSection &data);
  void operator()(const Funcall &funcall);
  void operator()(const ReturnValue &retval);
  void operator()(const Label &label);
  void operator()(const JmpIfZero &jz);
  void operator()(const Jmp &jmp);
  void operator()(const Store &store);
  void operator()(const Asm &assembly);
};
}; // namespace arm64_apple_darwin

class Generator_arm64_apple_darwin {
public:
  Generator_arm64_apple_darwin(const Compiler &compiler);
  std::string generate();

private:
  void generate_func(const Func &func);
  void generate_function_epilogue(const Func &func);
  void generate_function_prologue(const Func &func);
  void generate_stdlib();
  Compiler compiler;
  std::stringstream textstream;
  arm64_apple_darwin::Visitor visitor{textstream};
  bool is_main;
};

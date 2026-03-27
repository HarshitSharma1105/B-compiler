#include <Generator-arm64-apple-darwin.h>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <string>
// helper functions
int64_t to_i64(big_int x) {
  if (x < (big_int)INT64_MIN || x > (big_int)INT64_MAX) {
    errorf("integer overflow");
    exit(EXIT_FAILURE);
  }
  return (int64_t)x;
}
std::string to_64bithex(int64_t x) {
  std::stringstream buf;
  buf << std::hex << std::setw(16) << std::setfill('0')
      << static_cast<uint64_t>(x);
  return buf.str();
}
void load_const(const std::stringstream &stream, big_int lit) {
  int64_t x = to_i64(lit);
  auto hexval = to_64bithex(x);
  std::string tmp;
  stream << "    movz x10, #0x" << hexval.substr(12) << "\n";
  stream << "    movk x10, #0x" << hexval.substr(8, 4) << "\n";
  stream << "    movk x10, #0x" << hexval.substr(4, 4) << "\n";
  stream << "    movk x10, #0x" << hexval.substr(0, 4) << "\n";
}
// stack pointer : sp, base pointer : x29
void arm64_apple_darwin::ArgVisitor::operator()(const Var &var) {
  switch (var.type) {
  case Storage::Auto:
    stream << "    ldr x10, [x29, #" << (var.index + 1) * 8 << "]\n";
    break;
  }
case Storage::Global:
  stream << "    adrp x10, _" << var.var_name << "@PAGE\n"
         << "    ldr x10, [x10, _" << var.var_name << "@PAGEOFF\n";
  break;
case Storage::Array:
  stream << "    adrp x10, _" << var.var_name << "@PAGE\n"
         << "    add x10, x10, _" << var.var_name << "@PAGEOFF\n";
  break;
default:
  errorf("Undefined Argument passed in code generation\n");
}
void arm64_apple_darwin::ArgVisitor::operator()(const Literal &literal) {
  load_const(stream, literal.literal);
}
void arm64_apple_darwin::ArgVisitor::operator()(const DataOffset &data) {
  stream << "    adrp x10, data_" << data.start << "@PAGE\n"
         << "    add x10, x10, data_" << data.start << "@PAGEOFF\n";
  // stream << "    mov r15,data_" << data.start << "\n";
}
void arm64_apple_darwin::ArgVisitor::operator()(const FuncResult &funcresult) {
  stream << "    mov x10, x0\n";
}
void arm64_apple_darwin::ArgVisitor::operator()(const Ref &ref) {
  stream << "    ldr x10, [x29, #-" << (ref.index + 1) * 8 << "]\n";
  stream << "    ldr x10, [x10]\n";
}
void arm64_apple_darwin::ArgVisitor::operator()(const NoArg &noarg) {}

void arm64_apple_darwin::Visitor::operator()(const UnOp &unop) {
  if (unop.type == Tokentype::bit_and) {
    std::visit(
        overload{
            [&stream = stream](const Var &var) {
              switch (var.type) {
              case Storage::Auto:
                stream << "    add x10, x29, #-" << 8 * (var.index + 1) << "\n";
                break;
              case Storage::Global:
              case Storage::Array:
                stream << "    adrp x10, _" << var.var_name << "@PAGE\n"
                       << "    add x10, x10, _" << var.var_name << "@PAGEOFF\n";
                break;
              default:
                errorf("UNREACHABLE\n");
              }
            },
            [](const auto &) { errorf("TODO:Address of other variables"); }},
        unop.arg);
  } else {
    std::visit(argvisitor, unop.arg);
    stream << "    mov x9, x10\n";
    switch (unop.type) {
    case Tokentype::sub:
      stream << "    eor x10,x10,x10\n    sub x10, x10, x9\n";
      break;
    case Tokentype::not_:
      stream << "    cmp x9, #0\n    cset x10, eq\n";
      break;
    case Tokentype::bit_not:
      stream << "    mov x10,x9\n    mvn x10, x10\n";
      break;
    default:
      errorf("Unknown Unary Operation\n");
    }
  }
  stream << "    str x10, [x29, #-" << (unop.index + 1) * 8 << "]\n";
}
void arm64_apple_darwin::Visitor::operator()(const BinOp &binop) {
  std::visit(argvisitor, binop.rhs);
  stream << "    mov x9, x10\n";
  std::visit(argvisitor, binop.lhs);
  switch (binop.type) {
  case Tokentype::assignment:
    stream << "    mov x10,x9\n";
    break;
  case Tokentype::bit_and:
    stream << "    and x10, x10, x9\n";
    break;
  case Tokentype::bit_or:
    stream << "    orr x10, x10, x9\n";
    break;
  case Tokentype::less:
    stream << "    cmp x10, x9\n    cset x10, lt\n";
    break;
  case Tokentype::greater:
    stream << "    cmp x10, x9\n    cset x10, gt\n";
    break;
  case Tokentype::equals:
    stream << "    cmp x10, x9\n    cset x10, eq\n";
    break;
  case Tokentype::not_equals:
    stream << "    cmp x10, x9\n    cset x10, ne\n";
    break;
  case Tokentype::shift_left:
    stream << "    lsl x10, x10, x9\n";
    break;
  case Tokentype::shift_right:
    stream << "    lsr x10, x10, x9\n";
    break;
  case Tokentype::add:
    stream << "    add x10, x10, x9\n";
    break;
  case Tokentype::sub:
    stream << "    sub x10, x10, x9\n";
    break;
  case Tokentype::mult:
    stream << "    mul x10, x10, x9\n";
    break;
  case Tokentype::divi:
    stream << "    sdiv x10, x10, x9\n";
    break;
  case Tokentype::remainder:
    stream << "    sdiv x11, x10, x9\n"
           << "    msub x10, x11, x9, x10\n";
    break;
  default:
    errorf("Unknown Binary Operand type\n");
  }
  switch (binop.var.type) {
  case Storage::Auto:
    stream << "    str x10 [x29, #-" << (binop.var.index + 1) * 8 << "]\n";
    break;
  case Storage::Global:
    stream << "    adrp x11, _" << var.var_name << "@PAGE\n"
           << "    add x11, x11, _" << var.var_name << "@PAGEOFF\n";
    << "    str x10, [x11]\n";
    break;
  case Storage::Array:
    stream << "    adrp x11, _" << var.var_name << "@PAGE\n"
           << "    add x11, x11, _" << var.var_name << "@PAGEOFF\n";
    << "    ldr x12, [x11]\n" // loading the pointer to x12
    << "    str x10, [x12]\n";
    break;
  default:
    errorf("UNREACHABLE\n");
  }
}
void arm64_apple_darwin::Visitor::operator()(const Funcall &funcall) {
  assert(funcall.args.size() <= arm64_apple_darwin::regs.size() &&
         "too many args");
  for (size_t i = 0; i < funcall.args.size(); i++) {
    std::visit(argvisitor, funcall.args[i]);
    stream << "    mov " << arm64_apple_darwin::regs[i] << ", x10\n";
  }
  stream << "    eor x0, x0, x0\n";
  stream << "    bl _" << funcall.name << "\n";
}
void arm64_apple_darwin::Visitor::operator()(const DataSection &data) {
  stream << "section __DATA, __data\n";
  int count = 0, idx = 0;
  while (idx < data.concatedstrings.size()) {
    stream << "data_" << count++ << ":\n    .byte ";
    while (data.concatedstrings[idx] != '\n') {
      stream << data.concatedstrings[idx++];
    }
    idx++;
    stream << "\n";
  }
}
void arm64_apple_darwin::Visitor::operator()(const ReturnValue &retval) {
  std::visit(argvisitor, retval.arg);
  stream << "    mov x0,x10\n";
  stream << "    ldp x29, x30, [sp], #16\n"; // load reg pair
  stream << "    ret\n";
}
void arm64_apple_darwin::Visitor::operator()(const Label &label) {
  stream << "label_" << label.idx << ":\n";
}
void arm64_apple_darwin::Visitor::operator()(const JmpIfZero &jz) {
  std::visit(argvisitor, jz.arg);
  stream << "    cmp x10, #0\n";
  stream << "    beq label_" << jz.idx << "\n";
}
void arm64_apple_darwin::Visitor::operator()(const Jmp &jmp) {
  stream << "    b label_" << jmp.idx << "\n";
}
void arm64_apple_darwin::Visitor::operator()(const Store &store) {
  stream << "    ldr x9, [x29, -#" << (store.index + 1) * 8 << "]\n";
  std::visit(argvisitor, store.val);
  stream << "    str x10, [x9]\n";
}
void arm64_apple_darwin::Visitor::operator()(const Asm &assembly) {
  stream << assembly.asm_code << '\n';
}

Generator_arm64_apple_darwin::Generator_arm64_apple_darwin(
    const Compiler &compiler)
    : compiler(compiler) {}

std::string Generator_arm64_apple_darwin::generate() {
  for (const auto &name : compiler.extrns) {
    textstream << ".extrn " << name << "\n";
    textstream << ".globl _" << name << "\n_" << name << " = " << name
               << "\n"; // .globl _name _name = name
  }
  textstream << ".text\n.align 2\n";
  for (const auto &func : compiler.functions) {
    generate_function_prologue(func);
    generate_func(func);
    generate_function_epilogue(func);
  }
  std::visit(visitor, Op{DataSection{compiler.data_section}});
  textstream << ".data\n.align 3\n";
  for (const auto &[name, val] : compiler.globals) {
    textstream << ".globl _" << name << "\n"
               << "_" << name << ":\n    "
               << ".quad " << val << "\n";
  }
  textstream << ".bss\n.align 3\n";
  for (const auto &[name, size] : compiler.arrays) {
    textstream << ".globl _" << name << "\n"
               << "_" << name << ":\n    "
               << ".skip" << size << "\n";
  }
  return textstream.str();
}
void Generator_arm64_apple_darwin::generate_function_prologue(
    const Func &func) {
  assert(func.num_args <= 8 && "too many args");
  size_t alloc_size = func.max_vars_count;
  if (alloc_size % 2)
    alloc_size++;
  textstream << "_" << func.function_name << ":\n";
  if ((func.func_flags & Flag::AsmFunc) == 0) {
    textstream << "    stp x29, x30, [sp, #-16]\n";
    textstream << "    mov x29, sp\n";
    textstream << "    sub sp, sp, #" << 8 * alloc_size << "\n";
    for (int i = 0; i < func.num_args; i++) {
      textstream << "    str " << x86_64::regs[i] << ", [x29, #-" << (i + 1) * 8
                 << "]," << "\n";
    }
  }
}
void Generator_arm64_apple_darwin::generate_function_epilogue(
    const Func &func) {
  if ((func.func_flags & Flag::AsmFunc) == 0) {
    stream << "    ldp x29, x30, [sp], #16\n"; // load reg pair
    stream << "    ret\n";
  }
}
void Generator_arm64_apple_darwin64::generate_func(const Func &func) {
  for (const auto &op : func.function_body)
    std::visit(visitor, op);
}

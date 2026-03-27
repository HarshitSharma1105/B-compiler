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
void x86_64::ArgVisitor::operator()(const DataOffset &data) {
  stream << "    adrp x10, data_" << data.start << "@PAGE\n"
         << "    add x10, x10, data_" << data.start << "@PAGEOFF\n";
  // stream << "    mov r15,data_" << data.start << "\n";
}
void x86_64::ArgVisitor::operator()(const FuncResult &funcresult) {
  stream << "    mov x10, x0\n";
}
void x86_64::ArgVisitor::operator()(const Ref &ref) {
  stream << "    ldr x10, [x29, #-" << (ref.index + 1) * 8 << "]\n";
  stream << "    ldr x10, [x10]\n";
}
void x86_64::ArgVisitor::operator()(const NoArg &noarg) {}

#include "libskiff/generators/instruction_generator.hpp"
#include "libskiff/bytecode/floating_point.hpp"
#include "libskiff/bytecode/helpers.hpp"
#include "libskiff/bytecode/instructions.hpp"

#include <limits>

namespace libskiff {
namespace instructions {

instruction_generator_c::instruction_generator_c()
{
  _string_to_register["x0"] = 0x00;
  _string_to_register["x1"] = 0x01;
  _string_to_register["ip"] = 0x02;
  _string_to_register["sp"] = 0x03;
  _string_to_register["i0"] = 0x10;
  _string_to_register["i1"] = 0x11;
  _string_to_register["i2"] = 0x12;
  _string_to_register["i3"] = 0x13;
  _string_to_register["i4"] = 0x14;
  _string_to_register["i5"] = 0x15;
  _string_to_register["i6"] = 0x16;
  _string_to_register["i7"] = 0x17;
  _string_to_register["i8"] = 0x18;
  _string_to_register["i9"] = 0x19;
  _string_to_register["f0"] = 0x20;
  _string_to_register["f1"] = 0x21;
  _string_to_register["f2"] = 0x22;
  _string_to_register["f3"] = 0x23;
  _string_to_register["f4"] = 0x24;
  _string_to_register["f5"] = 0x25;
  _string_to_register["f6"] = 0x26;
  _string_to_register["f7"] = 0x27;
  _string_to_register["f8"] = 0x28;
  _string_to_register["f9"] = 0x29;
  _string_to_register["op"] = 0xFF;
}

void instruction_generator_c::update_meta(const uint64_t bytes)
{
  _instructions_generated++;
  _bytes_generated += bytes;
}

std::optional<std::vector<uint8_t>>
instruction_generator_c::gen_string_constant(const std::string_view str)
{
  bool padded{false};
  std::size_t len = str.size();

  if (len % 2 != 0) {
    len += 1;
    padded = true;
  }

  if (len > std::numeric_limits<uint16_t>::max()) {
    return std::nullopt;
  }

  // This encode function assumes the strings chars can be encoded within 1 byte
  // each
  std::vector<uint8_t> encoded_bytes;
  encoded_bytes.reserve(8 + len); // eight byte length encoding + string length
  auto encoded_size =
      libskiff::bytecode::helpers::pack_8(static_cast<uint64_t>(len));
  encoded_bytes.insert(encoded_bytes.end(), encoded_size.begin(),
                       encoded_size.end());

  for (auto &c : str) {
    encoded_bytes.push_back(static_cast<uint8_t>(c));
  }
  if (padded) {
    encoded_bytes.push_back(0x00);
  }
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::optional<std::vector<uint8_t>>
instruction_generator_c::gen_lib_section(const uint64_t address,
                                         const std::string section_name)
{
  std::vector<uint8_t> encoded_bytes;
  auto encoded_address = libskiff::bytecode::helpers::pack_8(address);
  auto encoded_section = gen_string_constant(section_name); // Same encoding

  if (encoded_section == std::nullopt) {
    return std::nullopt;
  }

  encoded_bytes.insert(encoded_bytes.end(), encoded_address.begin(),
                       encoded_address.end());
  encoded_bytes.insert(encoded_bytes.end(), encoded_section.value().begin(),
                       encoded_section.value().end());
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_u8_constant(const uint8_t value)
{
  std::vector<uint8_t> encoded_bytes = {0x00, value};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_u16_constant(const uint16_t value)
{
  auto encoded_bytes = libskiff::bytecode::helpers::pack_2(value);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_u32_constant(const uint32_t value)
{
  auto encoded_bytes = libskiff::bytecode::helpers::pack_4(value);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_u64_constant(const uint64_t value)
{
  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(value);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_i8_constant(const int8_t value)
{
  std::vector<uint8_t> encoded_bytes = {0x00, static_cast<uint8_t>(value)};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_i16_constant(const int16_t value)
{
  auto encoded_bytes =
      libskiff::bytecode::helpers::pack_2(static_cast<uint16_t>(value));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_i32_constant(const int32_t value)
{
  auto encoded_bytes =
      libskiff::bytecode::helpers::pack_4(static_cast<uint32_t>(value));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_i64_constant(const int64_t value)
{
  auto encoded_bytes =
      libskiff::bytecode::helpers::pack_8(static_cast<uint64_t>(value));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_fp_constant(const double value)
{
  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(
      libskiff::bytecode::floating_point::to_uint64_t(value));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::optional<uint8_t>
instruction_generator_c::get_register_value(const std::string &value)
{
  if (_string_to_register.find(value) == _string_to_register.end()) {
    return std::nullopt;
  }
  return _string_to_register[value];
}

std::vector<uint8_t> instruction_generator_c::gen_nop()
{
  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(
      static_cast<uint64_t>(bytecode::instructions::NOP));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_exit()
{
  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(
      static_cast<uint64_t>(bytecode::instructions::EXIT) << 32);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_blt(const uint8_t lhs,
                                                      const uint8_t rhs,
                                                      const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(lhs) << 16;
  top |= static_cast<uint32_t>(rhs) << 8;
  top |= static_cast<uint32_t>(bytecode::instructions::BLT);

  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_bgt(const uint8_t lhs,
                                                      const uint8_t rhs,
                                                      const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(lhs) << 16;
  top |= static_cast<uint32_t>(rhs) << 8;
  top |= static_cast<uint32_t>(bytecode::instructions::BGT);

  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_beq(const uint8_t lhs,
                                                      const uint8_t rhs,
                                                      const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(lhs) << 16;
  top |= static_cast<uint32_t>(rhs) << 8;
  top |= static_cast<uint32_t>(bytecode::instructions::BEQ);

  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_jmp(const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(bytecode::instructions::JMP);
  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_call(const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(bytecode::instructions::CALL);
  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_ret()
{
  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(
      static_cast<uint64_t>(bytecode::instructions::RET) << 32);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_mov(const uint8_t reg,
                                                      const uint32_t value)
{
  uint32_t top = static_cast<uint32_t>(reg);
  top <<= 8;
  top |= static_cast<uint32_t>(bytecode::instructions::MOV);
  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(value);

  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> generate_math_instruction(const uint8_t ins,
                                               const uint8_t dest,
                                               const uint8_t lhs,
                                               const uint8_t rhs)
{
  return {0x00, 0x00, 0x00, ins, dest, lhs, rhs, 0x00};
}

std::vector<uint8_t> instruction_generator_c::gen_add(const uint8_t dest,
                                                      const uint8_t lhs,
                                                      const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::ADD, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_sub(const uint8_t dest,
                                                      const uint8_t lhs,
                                                      const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::SUB, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_div(const uint8_t dest,
                                                      const uint8_t lhs,
                                                      const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::DIV, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_mul(const uint8_t dest,
                                                      const uint8_t lhs,
                                                      const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::MUL, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_addf(const uint8_t dest,
                                                       const uint8_t lhs,
                                                       const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::ADDF, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_subf(const uint8_t dest,
                                                       const uint8_t lhs,
                                                       const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::SUBF, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_divf(const uint8_t dest,
                                                       const uint8_t lhs,
                                                       const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::DIVF, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_mulf(const uint8_t dest,
                                                       const uint8_t lhs,
                                                       const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::MULF, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_lsh(const uint8_t dest,
                                                      const uint8_t lhs,
                                                      const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::LSH, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_rsh(const uint8_t dest,
                                                      const uint8_t lhs,
                                                      const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::RSH, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_and(const uint8_t dest,
                                                      const uint8_t lhs,
                                                      const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::AND, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_or(const uint8_t dest,
                                                     const uint8_t lhs,
                                                     const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::OR, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_xor(const uint8_t dest,
                                                      const uint8_t lhs,
                                                      const uint8_t rhs)
{
  auto encoded_bytes = generate_math_instruction(
      libskiff::bytecode::instructions::XOR, dest, lhs, rhs);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_not(const uint8_t dest,
                                                      const uint8_t source)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00,   0x00, libskiff::bytecode::instructions::NOT,
      dest, source, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_bltf(const uint8_t lhs,
                                                       const uint8_t rhs,
                                                       const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(lhs) << 16;
  top |= static_cast<uint32_t>(rhs) << 8;
  top |= static_cast<uint32_t>(bytecode::instructions::BLTF);

  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_bgtf(const uint8_t lhs,
                                                       const uint8_t rhs,
                                                       const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(lhs) << 16;
  top |= static_cast<uint32_t>(rhs) << 8;
  top |= static_cast<uint32_t>(bytecode::instructions::BGTF);

  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_beqf(const uint8_t lhs,
                                                       const uint8_t rhs,
                                                       const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(lhs) << 16;
  top |= static_cast<uint32_t>(rhs) << 8;
  top |= static_cast<uint32_t>(bytecode::instructions::BEQF);

  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = libskiff::bytecode::helpers::pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_asne(const uint8_t expected,
                                                       const uint8_t actual)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00,     0x00,   0x00, libskiff::bytecode::instructions::ASNE,
      expected, actual, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_aseq(const uint8_t expected,
                                                       const uint8_t actual)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00,     0x00,   0x00, libskiff::bytecode::instructions::ASEQ,
      expected, actual, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_push_w(const uint8_t source)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00,   0x00, 0x00, libskiff::bytecode::instructions::PUSH_W,
      source, 0x00, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_push_dw(const uint8_t source)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00,   0x00, 0x00, libskiff::bytecode::instructions::PUSH_DW,
      source, 0x00, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_push_qw(const uint8_t source)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00,   0x00, 0x00, libskiff::bytecode::instructions::PUSH_QW,
      source, 0x00, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_pop_w(const uint8_t dest)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00, 0x00, libskiff::bytecode::instructions::POP_W,
      dest, 0x00, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_pop_dw(const uint8_t dest)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00, 0x00, libskiff::bytecode::instructions::POP_DW,
      dest, 0x00, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_pop_qw(const uint8_t dest)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00, 0x00, libskiff::bytecode::instructions::POP_QW,
      dest, 0x00, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_alloc(const uint8_t dest,
                                                        const uint8_t source)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00,   0x00, libskiff::bytecode::instructions::ALLOC,
      dest, source, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_free(const uint8_t index)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00,  0x00, 0x00, libskiff::bytecode::instructions::FREE,
      index, 0x00, 0x00, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::gen_store_word(const uint8_t idx, const uint8_t offset,
                                        const uint8_t data)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00,   0x00, libskiff::bytecode::instructions::SW,
      idx,  offset, data, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_store_dword(
    const uint8_t idx, const uint8_t offset, const uint8_t data)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00,   0x00, libskiff::bytecode::instructions::SDW,
      idx,  offset, data, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_store_qword(
    const uint8_t idx, const uint8_t offset, const uint8_t data)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00,   0x00, libskiff::bytecode::instructions::SQW,
      idx,  offset, data, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::gen_load_word(const uint8_t idx, const uint8_t offset,
                                       const uint8_t dest)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00,   0x00, libskiff::bytecode::instructions::LW,
      idx,  offset, dest, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::gen_load_dword(const uint8_t idx, const uint8_t offset,
                                        const uint8_t dest)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00,   0x00, libskiff::bytecode::instructions::LDW,
      idx,  offset, dest, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::gen_load_qword(const uint8_t idx, const uint8_t offset,
                                        const uint8_t dest)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00,   0x00, libskiff::bytecode::instructions::LQW,
      idx,  offset, dest, 0x00};
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::gen_syscall(const uint32_t address)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00, 0x00, libskiff::bytecode::instructions::SYSCALL};
  auto encoded_address = libskiff::bytecode::helpers::pack_4(address);
  encoded_bytes.insert(encoded_bytes.end(), encoded_address.begin(),
                       encoded_address.end());
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_debug(const uint32_t address)
{
  std::vector<uint8_t> encoded_bytes = {
      0x00, 0x00, 0x00, libskiff::bytecode::instructions::DEBUG};
  auto encoded_address = libskiff::bytecode::helpers::pack_4(address);
  encoded_bytes.insert(encoded_bytes.end(), encoded_address.begin(),
                       encoded_address.end());
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> gen_eirq()
{
  return {0x00, 0x00, 0x00, libskiff::bytecode::instructions::EIRQ,
          0x00, 0x00, 0x00, 0x00};
}

std::vector<uint8_t> gen_dirq()
{
  return {0x00, 0x00, 0x00, libskiff::bytecode::instructions::DIRQ,
          0x00, 0x00, 0x00, 0x00};
}

} // namespace instructions
} // namespace libskiff

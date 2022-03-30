#include "libskiff/instruction_generator.hpp"
#include "libskiff/floating_point.hpp"
#include "libskiff/instructions.hpp"

#include <limits>

#include <bitset>
#include <iostream>

namespace libskiff {
namespace instructions {

instruction_generator_c::instruction_generator_c()
{

  _string_to_register["x0"] = 0x00;
  _string_to_register["x1"] = 0x01;
  _string_to_register["ip"] = 0x02;
  _string_to_register["fp"] = 0x03;
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
}

void instruction_generator_c::update_meta(const uint64_t bytes)
{
  _instructions_generated++;
  _bytes_generated += bytes;
}

std::optional<std::vector<uint8_t>>
instruction_generator_c::gen_string_constant(const std::string_view str)
{
  if (str.size() > std::numeric_limits<uint16_t>::max()) {
    return std::nullopt;
  }

  // This encode function assumes the strings chars can be encoded within 1 byte
  // each
  std::vector<uint8_t> encoded_bytes;
  encoded_bytes.reserve(2 +
                        str.size()); // two byte length encoding + string length

  uint16_t value = static_cast<uint16_t>(str.size());
  encoded_bytes.push_back(value & 0xFF00);
  encoded_bytes.push_back(value & 0x00FF);

  for (auto &c : str) {
    encoded_bytes.push_back(static_cast<uint8_t>(c));
  }
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

// These functions made purposely verbose

static inline std::vector<uint8_t> pack_2(const uint16_t value)
{
  std::vector<uint8_t> encoded_bytes;
  encoded_bytes.push_back(value & 0xFF00);
  encoded_bytes.push_back(value & 0x00FF);
  return encoded_bytes;
}

static inline std::vector<uint8_t> pack_4(const uint32_t value)
{
  std::vector<uint8_t> encoded_bytes;
  encoded_bytes.push_back(value & 0xFF000000);
  encoded_bytes.push_back(value & 0x00FF0000);
  encoded_bytes.push_back(value & 0x0000FF00);
  encoded_bytes.push_back(value & 0x000000FF);
  return encoded_bytes;
}

static inline std::vector<uint8_t> pack_8(const uint64_t value)
{
  std::vector<uint8_t> encoded_bytes;
  encoded_bytes.push_back(value & 0xFF00000000000000);
  encoded_bytes.push_back(value & 0x00FF000000000000);
  encoded_bytes.push_back(value & 0x0000FF0000000000);
  encoded_bytes.push_back(value & 0x000000FF00000000);
  encoded_bytes.push_back(value & 0x00000000FF000000);
  encoded_bytes.push_back(value & 0x0000000000FF0000);
  encoded_bytes.push_back(value & 0x000000000000FF00);
  encoded_bytes.push_back(value & 0x00000000000000FF);
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_u8_constant(const uint8_t value)
{
  std::vector<uint8_t> encoded_bytes(value);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_u16_constant(const uint16_t value)
{
  auto encoded_bytes = pack_2(value);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_u32_constant(const uint32_t value)
{
  auto encoded_bytes = pack_4(value);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_u64_constant(const uint64_t value)
{
  auto encoded_bytes = pack_8(value);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_i8_constant(const int8_t value)
{
  std::vector<uint8_t> encoded_bytes(static_cast<uint8_t>(value));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_i16_constant(const int16_t value)
{
  auto encoded_bytes = pack_2(static_cast<uint16_t>(value));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_i32_constant(const int32_t value)
{
  auto encoded_bytes = pack_4(static_cast<uint32_t>(value));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_i64_constant(const int64_t value)
{
  auto encoded_bytes = pack_8(static_cast<uint64_t>(value));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t>
instruction_generator_c::generate_fp_constant(const double value)
{
  auto encoded_bytes = pack_8(libskiff::floating_point::to_uint64_t(value));
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
  auto encoded_bytes = pack_8(static_cast<uint64_t>(instructions::NOP));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_exit()
{
  auto encoded_bytes = pack_8(static_cast<uint64_t>(instructions::EXIT) << 32);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_blt(const uint8_t lhs,
                                                      const uint8_t rhs,
                                                      const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(lhs) << 16;
  top |= static_cast<uint32_t>(rhs) << 8;
  top |= static_cast<uint32_t>(instructions::BLT);

  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_bgt(const uint8_t lhs,
                                                      const uint8_t rhs,
                                                      const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(lhs) << 16;
  top |= static_cast<uint32_t>(rhs) << 8;
  top |= static_cast<uint32_t>(instructions::BGT);

  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_beq(const uint8_t lhs,
                                                      const uint8_t rhs,
                                                      const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(lhs) << 16;
  top |= static_cast<uint32_t>(rhs) << 8;
  top |= static_cast<uint32_t>(instructions::BEQ);

  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_jmp(const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(instructions::JMP);
  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_call(const uint32_t address)
{
  uint32_t top = static_cast<uint32_t>(instructions::CALL);
  uint64_t ins = static_cast<uint64_t>(top) << 32;
  ins |= static_cast<uint64_t>(address);

  auto encoded_bytes = pack_8(ins);
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

std::vector<uint8_t> instruction_generator_c::gen_ret()
{
  auto encoded_bytes = pack_8(static_cast<uint64_t>(instructions::RET));
  update_meta(encoded_bytes.size());
  return encoded_bytes;
}

} // namespace instructions
} // namespace libskiff

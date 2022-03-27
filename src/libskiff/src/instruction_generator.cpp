#include "libskiff/instruction_generator.hpp"

#include <libskiff/floating_point.hpp>
#include <limits>

namespace libskiff {
namespace instructions {

instruction_generator_c::instruction_generator_c() {}

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

uint32_t instruction_generator_c::gen_nop() { return 0; }

} // namespace instructions
} // namespace libskiff

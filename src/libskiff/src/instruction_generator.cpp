#include "libskiff/instruction_generator.hpp"

#include "libskiff/coder.hpp"

#include <limits>

namespace libskiff {
namespace instructions {

instruction_generator_c::instruction_generator_c() {}

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

  auto encoded_length = coder::encode_16(str.size());
  encoded_bytes.insert(encoded_bytes.end(), encoded_length.begin(),
                       encoded_length.end());

  for (auto &c : str) {
    encoded_bytes.push_back(static_cast<uint8_t>(c));
  }
  return encoded_bytes;
}

uint32_t instruction_generator_c::gen_nop() { return 0; }

} // namespace instructions
} // namespace libskiff

#ifndef LIBSKIFF_CODER_HPP
#define LIBSKIFF_CODER_HPP

#include <cstdint>
#include <vector>

namespace libskiff {
namespace coder {

static inline std::vector<uint8_t> encode_16(const uint16_t value)
{
  std::vector<uint8_t> encoded;
  encoded.push_back(value & 0xFF00);
  encoded.push_back(value & 0x00FF);
  return encoded;
}

static inline uint16_t decode_16(std::vector<uint8_t> &data)
{
  uint16_t decoded = 0;
  for (auto &d : data) {
    decoded |= d;
  }
  return decoded;
}

} // namespace coder
} // namespace libskiff

#endif

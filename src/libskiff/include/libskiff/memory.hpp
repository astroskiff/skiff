#ifndef LIBSKIFF_MEMORY_HPP
#define LIBSKIFF_MEMORY_HPP

#include <cstdint>

namespace libskiff {
namespace memory {

constexpr uint8_t WORD_SIZE_BYTES = 2;  // 16-bit
constexpr uint8_t DWORD_SIZE_BYTES = 4; // 32-bits
constexpr uint8_t QWORD_SIZE_BYTES = 8; // 64-bits

} // namespace memory
} // namespace libskiff

#endif
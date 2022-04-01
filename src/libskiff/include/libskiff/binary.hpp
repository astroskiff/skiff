#ifndef LIBSKIFF_BINARY_HPP
#define LIBSKIFF_BINARY_HPP

#include <cstdint>

namespace libskiff {
namespace binary {
  
constexpr uint32_t compatibility = 0xFF'00'00'00; //! Indicate compatibility
constexpr uint8_t type_indicator_executable = 0xAA; //! Executable
constexpr uint8_t type_indicator_library = 0xFF;  //! Library

//! \brief The encoded constant types
enum class constant_type_e {
  U8 = 0x00,
  U16 = 0x01,
  U32 = 0x02,
  U64 = 0x03,
  I8 = 0x10,
  I16 = 0x11,
  I32 = 0x12,
  I64 = 0x13,
  FLOAT = 0x20,
  STRING = 0x30,
};

} // namespace binary
} // namespace libskiff

#endif

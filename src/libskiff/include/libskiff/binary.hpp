#ifndef LIBSKIFF_BINARY_HPP
#define LIBSKIFF_BINARY_HPP

#include <cstdint>

namespace libskiff {
namespace binary {
//! \brief Compatibility indicator
constexpr uint32_t compatibility = 0x00'00'00'00;

//! \brief The encoded constant types
enum class constant_type_e {
  U8,
  U16,
  U32,
  U64,
  I8,
  I16,
  I32,
  I64,
  FLOAT,
  STRING,
};

} // namespace binary
} // namespace libskiff

#endif

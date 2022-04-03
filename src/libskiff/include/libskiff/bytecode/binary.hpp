#ifndef LIBSKIFF_BINARY_HPP
#define LIBSKIFF_BINARY_HPP

#include <cstdint>

namespace libskiff {
namespace binary {

constexpr uint32_t compatibility = 0xFF'00'00'00;   //! Indicate compatibility
constexpr uint8_t type_indicator_executable = 0xAA; //! Executable
constexpr uint8_t type_indicator_library = 0xFF;    //! Library

} // namespace binary
} // namespace libskiff

#endif

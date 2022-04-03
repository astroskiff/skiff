#ifndef LIBSKIFF_TYPES_HPP
#define LIBSKIFF_TYPES_HPP

#include <cstdint>
#include <string>

namespace libskiff {
namespace types {

//! \brief Encoded constant types
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

//! \brief Execution-time debug level
enum class exec_debug_level_e {
  NONE = 0x00,
  MINIMAL = 0x01,
  MODERATE = 0x02,
  EXTREME = 0x03
};

//! \brief Semantic version
struct semver_t {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
};

//! \brief Dump a semver to a string
static std::string semver_to_string(const semver_t &s)
{
  return std::to_string(s.major) + "." + std::to_string(s.minor) + "." +
         std::to_string(s.patch);
}

} // namespace types
} // namespace libskiff

#endif
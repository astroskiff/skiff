#ifndef LIBSKIFF_TYPES_HPP
#define LIBSKIFF_TYPES_HPP

#include <cstdint>
#include <functional>
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

//! \brief Errors that can be thrown at runtime
enum class runtime_error_e { 
  RETURN_WITH_EMPTY_CALLSTACK,
  DIVIDE_BY_ZERO,
  ILLEGAL_INSTRUCTION,
  INSTRUCTION_PTR_OUT_OF_RANGE
   };

//! \brief Callback that will receive runtime errors
//!        generated by the VM
using runtime_error_cb = std::function<void(runtime_error_e)>;

//! \brief Register for vm
using vm_integer_reg = int64_t;
using vm_floating_point_reg = double;

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
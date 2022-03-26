#ifndef LIBSKIFF_INSTRUCTION_GENERATOR_HPP
#define LIBSKIFF_INSTRUCTION_GENERATOR_HPP

#include "instructions.hpp"

#include <cstdint>
#include <string_view>
#include <optional>
#include <vector>

namespace libskiff {
namespace instructions {

//! \brief Instruction generator
class instruction_generator_c {
public:
  instruction_generator_c();

  //! \brief Retrieve the number of instructions generated
  uint64_t get_number_instructions_generated() const
  {
    return _instructions_generated;
  }

  //! \brief Retrieve the number of bytes generated
  uint64_t get_number_bytes_generated() const
  {
    return _bytes_generated;
  }

  //! \brief Generate an encoded string constant 
  //! \returns Vector of bytes containing length as first 2 bytes with encoded string
  //!          data following. Returns std::nullopt iff length of string can be hel
  //!          within a 16-bit integer
  std::optional<std::vector<uint8_t>> gen_string_constant(const std::string_view str);

  //! \brief Generate nop instruction
  uint32_t gen_nop();

private:
  uint64_t _instructions_generated{0};
  uint64_t _bytes_generated{0};
};

} // namespace instructions
} // namespace libskiff

#endif

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

  //! \brief Generate the encoded integer constant
  std::vector<uint8_t> generate_u8_constant(const uint8_t value);

  //! \brief Generate the encoded integer constant
  std::vector<uint8_t> generate_u16_constant(const uint16_t value);

  //! \brief Generate the encoded integer constant
  std::vector<uint8_t> generate_u32_constant(const uint32_t value);

  //! \brief Generate the encoded integer constant
  std::vector<uint8_t> generate_u64_constant(const uint64_t value);

  //! \brief Generate the encoded integer constant
  std::vector<uint8_t> generate_i8_constant(const int8_t value);

  //! \brief Generate the encoded integer constant
  std::vector<uint8_t> generate_i16_constant(const int16_t value);

  //! \brief Generate the encoded integer constant
  std::vector<uint8_t> generate_i32_constant(const int32_t value);

  //! \brief Generate the encoded integer constant
  std::vector<uint8_t> generate_i64_constant(const int64_t value);

  //! \brief Generate the encoded floating point constant
  std::vector<uint8_t> generate_fp_constant(const double value);

  //! \brief Generate an encoded string constant 
  //! \returns Vector of bytes containing length as first 2 bytes with encoded string
  //!          data following. Returns std::nullopt iff length of string can be hel
  //!          within a 16-bit integer
  std::optional<std::vector<uint8_t>> gen_string_constant(const std::string_view str);

  //! \brief Generate nop instruction
  uint32_t gen_nop();

private:
  void update_meta(const uint64_t bytes_generated);
  uint64_t _instructions_generated{0};
  uint64_t _bytes_generated{0};
};

} // namespace instructions
} // namespace libskiff

#endif

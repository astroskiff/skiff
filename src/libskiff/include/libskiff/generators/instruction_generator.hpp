#ifndef LIBSKIFF_INSTRUCTION_GENERATOR_HPP
#define LIBSKIFF_INSTRUCTION_GENERATOR_HPP

#include "libskiff/bytecode/instructions.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
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
  uint64_t get_number_bytes_generated() const { return _bytes_generated; }

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

  //! \brief Attempt to convert a string to a register value
  //! \returns std::nullopt if the string doesn't contain a valid
  //!          register
  std::optional<uint8_t> get_register_value(const std::string &value);

  //! \brief Generate an encoded string constant
  //! \returns Vector of bytes containing length as first 8 bytes with encoded
  //! string
  //!          data following. Returns std::nullopt iff length of string can be
  //!          held within a 64-bit integer
  std::optional<std::vector<uint8_t>>
  gen_string_constant(const std::string_view str);

  //! \brief Generate encoded library section
  //! \note  Returns std::nullopt iff length of string can be
  //!          held within a 64-bit integer
  std::optional<std::vector<uint8_t>>
  gen_lib_section(const uint64_t address, const std::string section_name);

  //! \brief Generate nop instruction
  std::vector<uint8_t> gen_nop();

  //! \brief Generate exit instruction
  std::vector<uint8_t> gen_exit();

  //! \brief Generate blt instruction
  std::vector<uint8_t> gen_blt(const uint8_t lhs, const uint8_t rhs,
                               const uint32_t address);

  //! \brief Generate bgt instruction
  std::vector<uint8_t> gen_bgt(const uint8_t lhs, const uint8_t rhs,
                               const uint32_t address);

  //! \brief Generate beq instruction
  std::vector<uint8_t> gen_beq(const uint8_t lhs, const uint8_t rhs,
                               const uint32_t address);

  //! \brief Generate jmp instruction
  std::vector<uint8_t> gen_jmp(const uint32_t address);

  //! \brief Generate call instruction
  std::vector<uint8_t> gen_call(const uint32_t address);

  //! \brief Generate ret instruction
  std::vector<uint8_t> gen_ret();

  //! \brief Generate mov instruction
  std::vector<uint8_t> gen_mov(const uint8_t reg, const uint32_t value);

  //! \brief Generate add instruction
  std::vector<uint8_t> gen_add(const uint8_t dest, const uint8_t lhs,
                               const uint8_t rhs);

  //! \brief Generate sub instruction
  std::vector<uint8_t> gen_sub(const uint8_t dest, const uint8_t lhs,
                               const uint8_t rhs);

  //! \brief Generate div instruction
  std::vector<uint8_t> gen_div(const uint8_t dest, const uint8_t lhs,
                               const uint8_t rhs);

  //! \brief Generate mul instruction
  std::vector<uint8_t> gen_mul(const uint8_t dest, const uint8_t lhs,
                               const uint8_t rhs);

  //! \brief Generate addf instruction
  std::vector<uint8_t> gen_addf(const uint8_t dest, const uint8_t lhs,
                                const uint8_t rhs);

  //! \brief Generate subf instruction
  std::vector<uint8_t> gen_subf(const uint8_t dest, const uint8_t lhs,
                                const uint8_t rhs);

  //! \brief Generate divf instruction
  std::vector<uint8_t> gen_divf(const uint8_t dest, const uint8_t lhs,
                                const uint8_t rhs);

  //! \brief Generate mulf instruction
  std::vector<uint8_t> gen_mulf(const uint8_t dest, const uint8_t lhs,
                                const uint8_t rhs);

  //! \brief Generate lsh instruction
  std::vector<uint8_t> gen_lsh(const uint8_t dest, const uint8_t lhs,
                               const uint8_t rhs);

  //! \brief Generate rsh instruction
  std::vector<uint8_t> gen_rsh(const uint8_t dest, const uint8_t lhs,
                               const uint8_t rhs);

  //! \brief Generate and instruction
  std::vector<uint8_t> gen_and(const uint8_t dest, const uint8_t lhs,
                               const uint8_t rhs);

  //! \brief Generate or instruction
  std::vector<uint8_t> gen_or(const uint8_t dest, const uint8_t lhs,
                              const uint8_t rhs);

  //! \brief Generate xor instruction
  std::vector<uint8_t> gen_xor(const uint8_t dest, const uint8_t lhs,
                               const uint8_t rhs);

  //! \brief Generate not instruction
  std::vector<uint8_t> gen_not(const uint8_t dest, const uint8_t source);

private:
  void update_meta(const uint64_t bytes_generated);
  uint64_t _instructions_generated{0};
  uint64_t _bytes_generated{0};
  std::unordered_map<std::string, uint8_t> _string_to_register;
};

} // namespace instructions
} // namespace libskiff

#endif
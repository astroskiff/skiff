#ifndef LIBSKIFF_BINARY_GENERATOR_HPP
#define LIBSKIFF_BINARY_GENERATOR_HPP

#include <cstdint>
#include <vector>

#include "binary.hpp"

namespace libskiff {
namespace binary {
//! \brief The binary generator
class generator_c {
public:
  generator_c();

  //! \brief Add a constant encoded from the instruction_generator
  //! \param data Encoded constant
  //! \returns Address assigned to constant
  uint64_t add_constant(const constant_type_e type,
                        const std::vector<uint8_t> data);

  //! \brief Add an instruction made by the instruction_generator
  //!        to the binary
  //! \param instruction The instruction to add to the binary
  void add_instruction(const std::vector<uint8_t> instruction);

  //! \brief Generate the binary with the data given thus far
  //! \returns A skiff binary that will be compliant with the
  //!          compatibility constant
  std::vector<uint8_t> generate_binary() const;

private:
  std::vector<uint8_t> _constant_data;
  std::vector<uint8_t> _instruction_data;
  uint64_t _number_of_constants{0};
  uint64_t _constant_address{0};
};
} // namespace binary
} // namespace libskiff

#endif

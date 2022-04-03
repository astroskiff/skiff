#ifndef LIBSKIFF_ASSEMBLE_HPP
#define LIBSKIFF_ASSEMBLE_HPP

#include <optional>
#include <string>
#include <vector>

#include "libskiff/types.hpp"

namespace libskiff {
namespace assembler {

//! \brief Statistics about the assemble process
struct stats_t {
  uint64_t num_instructions; //! Number of instructions generated
};

//! \brief Assembler output
struct assembled_t {
  stats_t stats;                                    //! Stats
  std::optional<std::vector<std::string>> errors;   //! Errors produced
  std::optional<std::vector<std::string>> warnings; //! Warnings produced
  std::optional<std::vector<uint8_t>> bin;          //! Resulting binary
};

//! \brief Assemble
extern assembled_t assemble(const std::string &input);

} // namespace assembler
} // namespace libskiff

#endif

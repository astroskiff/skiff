#ifndef SKIFF_APP_ASSEMBLE_HPP
#define SKIFF_APP_ASSEMBLE_HPP

#include <optional>
#include <string>
#include <vector>

namespace skiff_assemble {

struct stats_t {
  uint64_t num_instructions;
};

struct assembled_t {
  stats_t stats;
  std::optional<std::vector<std::string>> errors;
  std::optional<std::vector<std::string>> warnings;
  std::optional<std::vector<uint8_t>> bin;
};

extern assembled_t assemble(const std::string &input);

} // namespace skiff_assemble

#endif

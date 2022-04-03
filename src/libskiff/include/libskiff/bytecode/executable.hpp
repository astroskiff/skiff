#ifndef LIBSKIFF_EXECUTABLE_HPP
#define LIBSKIFF_EXECUTABLE_HPP

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "libskiff/types.hpp"

namespace libskiff {
namespace binary {

//! \brief A loaded item
class executable_c {
public:
  executable_c() : _debug_level(libskiff::types::exec_debug_level_e::NONE){};

  //! \brief Construct an executable item
  executable_c(const uint32_t compatibility)
      : _compatiblity_dword(compatibility)
  {
  }

  //! \brief Returns true if loaded object was marked as experimental
  bool is_experimental() const { return (_compatiblity_dword >> 24) == 0xFF; }

  //! \brief Retrieve the semver baked into the binary
  libskiff::types::semver_t bytecode_semver() const { return _semver; }

  //! \brief Set the debug level
  void set_debug_level(libskiff::types::exec_debug_level_e level)
  {
    _debug_level = level;
  }

  //! \brief Set constants
  void set_constants(const std::vector<uint8_t> &c) { _constants = c; }

  //! \brief Set instructions
  void set_instructions(const std::vector<uint8_t> &c) { _instructions = c; }

  //! \brief Set the entry address to the executable
  void set_entry_address(const uint64_t address) { _entry_address = address; }

  //! \brief Adds a section and address
  //! \returns false iff the name is not unique
  bool add_section(const std::string &name, const uint64_t address);

  //! \brief Retrieve the debug level for the executable
  libskiff::types::exec_debug_level_e get_debug_level() const
  {
    return _debug_level;
  }

  //! \brief Get the entry address
  uint64_t get_entry_address() const { return _entry_address; }

  //! \brief Attempt to get a section address from the library table
  //! \returns nullopt if the name
  std::optional<uint64_t> get_section_address(const std::string &name) const;

private:
  std::vector<uint8_t> _constants;
  std::vector<uint8_t> _instructions;
  uint32_t _compatiblity_dword;
  libskiff::types::semver_t _semver;
  uint64_t _entry_address{0};
  std::unordered_map<std::string, uint64_t> _section_table;
  libskiff::types::exec_debug_level_e _debug_level;
};

//! \brief Load a given binary
//! \param file The file to load
//! \returns Optional unique pointer to a executable_c object. If there are
//!          errors loading the file nullopt will be returned
std::optional<std::unique_ptr<executable_c>>
load_binary(const std::string &file);

} // namespace binary
} // namespace libskiff

#endif

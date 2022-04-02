#ifndef LIBSKIFF_LOADER_HPP
#define LIBSKIFF_LOADER_HPP

#include <cstdint>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>

#include "libskiff/types.hpp"

namespace libskiff {
namespace binary {

//! \brief A loaded item
class loaded {
public:
  //! \brief Construct a the loaded item with the type marked
  loaded(libskiff::types::binary_type_e type) : _type(type){}
  virtual ~loaded() = default;

  //! \brief Retrieve the loaded object's type
  libskiff::types::binary_type_e get_type() const { return _type; }

  //! \brief Returns true if loaded object was marked as experimental
  bool is_experimental() const { return (_compatiblity_dword >> 24) == 0xFF; }

  //! \brief Retrieve the semver baked into the binary
  libskiff::types::semver_t bytecode_semver() const { return _semver; }

protected:
  libskiff::types::binary_type_e _type;
  std::vector<uint8_t> _constants;
  std::vector<uint8_t> _instructions;
  uint32_t _compatiblity_dword;
  libskiff::types::semver_t _semver;
};

//! \brief An executable object
class executable_c : public loaded {
public:
  executable_c() : loaded(libskiff::types::binary_type_e::EXECUTABLE) {};
};

//! \brief A library object
class library_c : public loaded {
public:
  library_c() : loaded(libskiff::types::binary_type_e::LIBRARY) {};

  //! \brief Attempt to get a section address from the library table
  //! \returns nullopt if the name 
  std::optional<uint64_t> get_section_address(const std::string& name) const;

private:
  std::unordered_map<std::string, uint64_t> _section_table;
};

//! \brief Load a given binary
//! \param file The file to load
//! \returns Optional unique pointer to a loaded object. If there are
//!          errors loading the file nullopt will be returned
std::optional<std::unique_ptr<loaded>> load_binary(const std::string& file);

} // namespace binary
} // namespace libskiff

#endif

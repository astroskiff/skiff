#include "libskiff/bytecode/loader.hpp"

#include <filesystem>
#include <fstream>

#include "libskiff/logging/aixlog.hpp"

namespace libskiff {
namespace binary {


std::optional<std::unique_ptr<loaded>> load_binary(const std::string& file)
{
  if(!std::filesystem::is_regular_file(file)) {
    return std::nullopt;
  }



}

std::optional<uint64_t> library_c::get_section_address(const std::string& name) const
{
  if(_section_table.find(name) == _section_table.end()) {
    return std::nullopt;
  }
  return _section_table.at(name);
}

}
}
#include "libskiff/bytecode/executable.hpp"
#include "libskiff/bytecode/helpers.hpp"
#include "libskiff/logging/aixlog.hpp"
#include "libskiff/types.hpp"

#include <filesystem>
#include <fstream>

namespace libskiff {
namespace bytecode {

namespace {

inline std::vector<uint8_t> read(std::ifstream &is, const std::streamsize len)
{
  //  std::cout << ">>>>File location: " << is.tellg() << ", data: ";

  std::vector<uint8_t> in(len);
  is.read(reinterpret_cast<char *>(in.data()), len);

  /*
    if(in.size() == 1) { std::cout << (int)in[0]; }
    if(in.size() == 2) { std::cout <<
    libskiff::bytecode::helpers::unpack_2(in).value(); } if(in.size() == 4) {
    std::cout << libskiff::bytecode::helpers::unpack_4(in).value(); }
    if(in.size() == 8) { std::cout <<
    libskiff::bytecode::helpers::unpack_8(in).value(); } std::cout << "\n";
  */

  return in;
}

} // namespace

std::optional<std::unique_ptr<executable_c>>
load_binary(const std::string &file)
{
  LOG(TRACE) << TAG("loader") << __func__ << "\n";
  if (!std::filesystem::is_regular_file(file)) {
    LOG(FATAL) << TAG("loader") << "Unable to load file : " << file << "\n";
    return std::nullopt;
  }

  if (std::ifstream is{file, std::ios::binary}) {

    // Read compatibility DWORD
    auto compatiblity = libskiff::bytecode::helpers::unpack_4(read(is, 4));
    if (compatiblity == std::nullopt) {
      LOG(FATAL) << TAG("loader")
                 << "Unable to read compatibility DWORD : " << file << "\n";
      return std::nullopt;
    }

    executable_c *loaded_object = new executable_c(*compatiblity);

    // Debug Level
    auto dlevel = read(is, 1)[0];
    auto dbg_level = static_cast<libskiff::types::exec_debug_level_e>(dlevel);
    switch (dbg_level) {
    case libskiff::types::exec_debug_level_e::NONE:
      [[fallthrough]];
    case libskiff::types::exec_debug_level_e::MINIMAL:
      [[fallthrough]];
    case libskiff::types::exec_debug_level_e::MODERATE:
      [[fallthrough]];
    case libskiff::types::exec_debug_level_e::EXTREME:
      break;
    default:
      LOG(FATAL) << TAG("loader") << "Invalid debug level\n";
      return std::nullopt;
    }
    LOG(DEBUG) << TAG("loader") << "Setting debug to level : " << (int)dlevel
               << "\n";
    loaded_object->set_debug_level(dbg_level);

    // Section table num entries
    auto num_entries = libskiff::bytecode::helpers::unpack_8(read(is, 8));
    if (num_entries == std::nullopt) {
      LOG(FATAL) << TAG("loader")
                 << "Failed to read in number of section table entries\n";
      return std::nullopt;
    }

    LOG(DEBUG) << TAG("loader")
               << "Section table entries : " << num_entries.value() << "\n";

    // Read in section table
    for (auto i = 0; i < num_entries.value(); i++) {

      // Section Address
      auto saddr = libskiff::bytecode::helpers::unpack_8(read(is, 8));
      if (saddr == std::nullopt) {
        LOG(FATAL) << TAG("loader") << "Failed to read in section address\n";
        return std::nullopt;
      }

      // Section Length
      auto slen = libskiff::bytecode::helpers::unpack_8(read(is, 8));
      if (slen == std::nullopt) {
        LOG(FATAL) << TAG("loader") << "Failed to read in section length\n";
        return std::nullopt;
      }

      // Section Name
      std::vector<uint8_t> sname = read(is, slen.value());

      std::string s;
      for (auto &i : sname) {
        s += static_cast<char>(i);
      }

      if (!loaded_object->add_section(s, saddr.value())) {
        LOG(FATAL) << TAG("loader") << "Unable to section: " << s
                   << " to table\n";
        return std::nullopt;
      }

      LOG(DEBUG) << TAG("loader") << "Loaded section '" << s
                 << "' at address :" << saddr.value() << "\n";
    }

    // Interrupt table num interrupts
    auto interrupt_table_num_entries =
        libskiff::bytecode::helpers::unpack_8(read(is, 8));
    if (num_entries == std::nullopt) {
      LOG(FATAL) << TAG("loader")
                 << "Failed to read in number of interrupt table entries\n";
      return std::nullopt;
    }

    LOG(DEBUG) << TAG("loader")
               << "Interrupt table entries : " << num_entries.value() << "\n";

    // Read in interrupt table
    for (auto i = 0; i < interrupt_table_num_entries.value(); i++) {

      // Interrupt Number
      auto interrupt_number =
          libskiff::bytecode::helpers::unpack_8(read(is, 8));
      if (interrupt_number == std::nullopt) {
        LOG(FATAL) << TAG("loader") << "Failed to read in interrupt number\n";
        return std::nullopt;
      }

      // Interrupt Address
      auto interrupt_address =
          libskiff::bytecode::helpers::unpack_8(read(is, 8));
      if (interrupt_address == std::nullopt) {
        LOG(FATAL) << TAG("loader") << "Failed to read in interrupt address\n";
        return std::nullopt;
      }

      if (!loaded_object->add_interrupt(*interrupt_number,
                                        *interrupt_address)) {
        LOG(FATAL)
            << TAG("loader")
            << "Failed to add interrupt to map -> Non unique number given\n";
        return std::nullopt;
      }
    }

    // Read constant count QWORD
    auto const_count = libskiff::bytecode::helpers::unpack_8(read(is, 8));
    if (const_count == std::nullopt) {
      LOG(FATAL) << TAG("loader") << "Unable to read constant count QWORD\n";
      return std::nullopt;
    }

    std::vector<uint8_t> constants;
    constants.reserve(const_count.value());

    // Read constants
    for (auto i = 0; i < const_count.value(); i++) {
      LOG(DEBUG) << TAG("loader") << "Read constant " << i + 1 << " of "
                 << const_count.value() << "\n";

      // Read 1 - Constant encoding
      auto id = read(is, 1);
      if (id.empty()) {
        LOG(FATAL) << TAG("loader") << "Unable to read id BYTE\n";
      }

      switch (static_cast<libskiff::types::constant_type_e>(id[0])) {

      // Constants are word aligned so U8 and I8 are actually 2 bytes
      case libskiff::types::constant_type_e::U8:
        [[fallthrough]];
      case libskiff::types::constant_type_e::I8:
        [[fallthrough]];
      case libskiff::types::constant_type_e::U16:
        [[fallthrough]];
      case libskiff::types::constant_type_e::I16: {
        auto value = read(is, 2);
        constants.insert(constants.end(), value.begin(), value.end());
        break;
      }
      case libskiff::types::constant_type_e::U32:
        [[fallthrough]];
      case libskiff::types::constant_type_e::I32: {
        auto value = read(is, 4);
        constants.insert(constants.end(), value.begin(), value.end());
        break;
      }
      case libskiff::types::constant_type_e::U64:
        [[fallthrough]];
      case libskiff::types::constant_type_e::I64:
        [[fallthrough]];
      case libskiff::types::constant_type_e::FLOAT: {
        auto value = read(is, 8);
        constants.insert(constants.end(), value.begin(), value.end());
        break;
      }
      case libskiff::types::constant_type_e::STRING: {
        auto str_len = libskiff::bytecode::helpers::unpack_8(read(is, 8));
        if (str_len == std::nullopt) {
          LOG(FATAL) << TAG("loader") << "Unable to read const string len\n";
          return std::nullopt;
        }
        std::vector<uint8_t> str = read(is, str_len.value());
        constants.insert(constants.end(), str.begin(), str.end());
        break;
      }
      default:
        LOG(FATAL) << TAG("loader")
                   << "Invalid const id given for encoded constant\n";
        return std::nullopt;
      }
    }

    loaded_object->set_constants(constants);

    // Read in entry address
    auto entry = libskiff::bytecode::helpers::unpack_8(read(is, 8));
    if (entry == std::nullopt) {
      LOG(FATAL) << TAG("loader") << "Failed to read in entry address\n";
      return std::nullopt;
    }

    loaded_object->set_entry_address(entry.value());

    // Read in number of instruction bytes
    auto num_instructions = libskiff::bytecode::helpers::unpack_8(read(is, 8));
    if (num_instructions == std::nullopt) {
      LOG(FATAL) << TAG("loader")
                 << "Failed to read in number of instructions\n";
      return std::nullopt;
    }

    // Read in all of the instructions
    std::vector<uint8_t> instructions;
    {
      auto instruction = read(is, *num_instructions);
      instructions.insert(instructions.end(), instruction.begin(),
                          instruction.end());
    }

    loaded_object->set_instructions(instructions);
    return std::unique_ptr<executable_c>(loaded_object);
  }
  return std::nullopt;
}

std::optional<uint64_t>
executable_c::get_section_address(const std::string &name) const
{
  if (_section_table.find(name) == _section_table.end()) {
    return std::nullopt;
  }
  return _section_table.at(name);
}

bool executable_c::add_section(const std::string &name, const uint64_t addr)
{
  if (_section_table.find(name) != _section_table.end()) {
    return false;
  }
  _section_table[name] = addr;
  return true;
}

bool executable_c::add_interrupt(const uint64_t id, const uint64_t address)
{
  if (_interrupt_number_to_address.find(id) !=
      _interrupt_number_to_address.end()) {
    return false;
  }
  _interrupt_number_to_address[id] = address;
  return true;
}

} // namespace bytecode
} // namespace libskiff
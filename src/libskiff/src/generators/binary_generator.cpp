#include "libskiff/generators/binary_generator.hpp"
#include "libskiff/bytecode/helpers.hpp"

namespace libskiff {
namespace generator {

uint64_t
binary_generator::add_constant(const libskiff::types::constant_type_e type,
                               const std::vector<uint8_t> data)
{
  _constant_data.push_back(static_cast<uint8_t>(type));

  // Ensure address doesn't include the byte required to load the thing for the
  // vm
  uint64_t address = _constant_address;
  _constant_data.insert(_constant_data.end(), data.begin(), data.end());
  _number_of_constants++;
  _constant_address += data.size();
  return address;
}

void binary_generator::add_instruction(const std::vector<uint8_t> instruction)
{
  _instruction_data.insert(_instruction_data.end(), instruction.begin(),
                           instruction.end());
  _number_of_instructions++;
}

void binary_generator::set_entry(const uint64_t address) { _entry = address; }

void binary_generator::set_debug(
    const libskiff::types::exec_debug_level_e level)
{
  _debug = level;
}

std::vector<uint8_t> binary_generator::generate_binary() const
{
  auto encoded_compatibility =
      libskiff::bytecode::helpers::pack_4(libskiff::binary::compatibility);
  auto encoded_number_of_constants =
      libskiff::bytecode::helpers::pack_8(_number_of_constants);
  auto encoded_number_of_sections =
      libskiff::bytecode::helpers::pack_8(_num_sections);
  auto encoded_entry = libskiff::bytecode::helpers::pack_8(_entry);
  auto encoded_number_instructions =
      libskiff::bytecode::helpers::pack_8(_number_of_instructions);

  std::vector<uint8_t> binary;
  // Compatibility DWORD
  binary.insert(binary.end(), encoded_compatibility.begin(),
                encoded_compatibility.end());

  // Debug level
  binary.push_back(static_cast<uint8_t>(_debug));

  // Section Table
  binary.insert(binary.end(), encoded_number_of_sections.begin(),
                encoded_number_of_sections.end());
  binary.insert(binary.end(), _section_table.begin(), _section_table.end());

  // Constants
  binary.insert(binary.end(), encoded_number_of_constants.begin(),
                encoded_number_of_constants.end());
  binary.insert(binary.end(), _constant_data.begin(), _constant_data.end());

  // Entry QWORD
  binary.insert(binary.end(), encoded_entry.begin(), encoded_entry.end());

  // Num Instructions QWORD
  binary.insert(binary.end(), encoded_number_instructions.begin(),
                encoded_number_instructions.end());

  // Instructuns
  binary.insert(binary.end(), _instruction_data.begin(),
                _instruction_data.end());
  return binary;
}

void binary_generator::add_section(std::vector<uint8_t> encoded_section)
{
  _num_sections++;
  _section_table.insert(_section_table.end(), encoded_section.begin(),
                        encoded_section.end());
}

} // namespace generator
} // namespace libskiff
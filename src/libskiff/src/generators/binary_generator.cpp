#include "libskiff/generators/binary_generator.hpp"

namespace libskiff {
namespace generator {

namespace {

static inline std::vector<uint8_t> pack_4(const uint32_t value)
{
  std::vector<uint8_t> encoded_bytes;
  encoded_bytes.push_back(value >> 24);
  encoded_bytes.push_back(value >> 16);
  encoded_bytes.push_back(value >> 8);
  encoded_bytes.push_back(value);
  return encoded_bytes;
}

static inline std::vector<uint8_t> pack_8(const uint64_t value)
{
  std::vector<uint8_t> encoded_bytes;
  encoded_bytes.push_back(value >> 56);
  encoded_bytes.push_back(value >> 48);
  encoded_bytes.push_back(value >> 40);
  encoded_bytes.push_back(value >> 32);
  encoded_bytes.push_back(value >> 24);
  encoded_bytes.push_back(value >> 16);
  encoded_bytes.push_back(value >> 8);
  encoded_bytes.push_back(value);
  return encoded_bytes;
}

} // namespace

executable_c::executable_c() {}

uint64_t executable_c::add_constant(const binary::constant_type_e type,
                                    const std::vector<uint8_t> data)
{
  _constant_data.push_back(static_cast<uint8_t>(type));

  // Ensure address doesn't include the byte required to load the thing for the
  // vm
  uint64_t address = _constant_address;
  _constant_data.insert(_constant_data.end(), _constant_data.begin(),
                        _constant_data.end());
  _number_of_constants++;
  _constant_address += data.size();
  return address;
}

void executable_c::add_instruction(const std::vector<uint8_t> instruction)
{
  _instruction_data.insert(_instruction_data.end(), _instruction_data.begin(),
                           _instruction_data.end());
}

void executable_c::set_entry(const uint64_t address) { _entry = address; }

std::vector<uint8_t> executable_c::generate_binary() const
{
  auto encoded_compatibility = pack_4(libskiff::binary::compatibility);
  auto encoded_number_of_constants = pack_8(_number_of_constants);
  auto encoded_entry = pack_8(_entry);

  std::vector<uint8_t> binary;
  binary.insert(binary.end(), encoded_compatibility.begin(),
                encoded_compatibility.end());
  binary.push_back(libskiff::binary::type_indicator_executable);
  binary.insert(binary.end(), encoded_number_of_constants.begin(),
                encoded_number_of_constants.end());
  binary.push_back(0xFF);
  binary.insert(binary.end(), _constant_data.begin(), _constant_data.end());
  binary.push_back(0xFF);
  binary.insert(binary.end(), encoded_entry.begin(), encoded_entry.end());
  binary.insert(binary.end(), _instruction_data.begin(),
                _instruction_data.end());
  return binary;
}

library_c::library_c() {}

uint64_t library_c::add_constant(const binary::constant_type_e type,
                                 const std::vector<uint8_t> data)
{
  _constant_data.push_back(static_cast<uint8_t>(type));

  // Ensure address doesn't include the byte required to load the thing for the
  // vm
  uint64_t address = _constant_address;
  _constant_data.insert(_constant_data.end(), _constant_data.begin(),
                        _constant_data.end());
  _number_of_constants++;
  _constant_address += data.size();
  return address;
}

void library_c::add_instruction(const std::vector<uint8_t> instruction)
{
  _instruction_data.insert(_instruction_data.end(), _instruction_data.begin(),
                           _instruction_data.end());
}

void library_c::add_section(std::vector<uint8_t> encoded_section)
{
  _section_table.insert(_section_table.end(), encoded_section.begin(),
                        encoded_section.end());
}

std::vector<uint8_t> library_c::generate_binary() const
{
  auto encoded_compatibility = pack_4(libskiff::binary::compatibility);
  auto encoded_number_of_constants = pack_8(_number_of_constants);
  std::vector<uint8_t> binary;
  binary.insert(binary.end(), encoded_compatibility.begin(),
                encoded_compatibility.end());
  binary.push_back(libskiff::binary::type_indicator_library);
  binary.insert(binary.end(), encoded_number_of_constants.begin(),
                encoded_number_of_constants.end());
  binary.push_back(0xFF);
  binary.insert(binary.end(), _constant_data.begin(), _constant_data.end());
  binary.push_back(0xFF);
  binary.insert(binary.end(), _section_table.begin(), _section_table.end());
  binary.push_back(0xFF);
  binary.insert(binary.end(), _instruction_data.begin(),
                _instruction_data.end());
  return binary;
}

} // namespace generator
} // namespace libskiff
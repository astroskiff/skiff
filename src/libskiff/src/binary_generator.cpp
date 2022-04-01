#include "libskiff/binary_generator.hpp"

namespace libskiff {
namespace binary {

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

generator_c::generator_c() {}

uint64_t generator_c::add_constant(const constant_type_e type,
                                   const std::vector<uint8_t> data)
{
  switch (type) {
  case constant_type_e::U8:
    _constant_data.push_back(0x00);
    break;
  case constant_type_e::U16:
    _constant_data.push_back(0x01);
    break;
  case constant_type_e::U32:
    _constant_data.push_back(0x02);
    break;
  case constant_type_e::U64:
    _constant_data.push_back(0x03);
    break;
  case constant_type_e::I8:
    _constant_data.push_back(0x10);
    break;
  case constant_type_e::I16:
    _constant_data.push_back(0x11);
    break;
  case constant_type_e::I32:
    _constant_data.push_back(0x12);
    break;
  case constant_type_e::I64:
    _constant_data.push_back(0x13);
    break;
  case constant_type_e::FLOAT:
    _constant_data.push_back(0x20);
    break;
  case constant_type_e::STRING:
    _constant_data.push_back(0x30);
    break;
  }
  // Ensure address doesn't include the byte required to load the thing for the
  // vm
  uint64_t address = _constant_address;
  _constant_data.insert(_constant_data.end(), _constant_data.begin(),
                        _constant_data.end());
  _number_of_constants++;
  _constant_address += data.size();
  return address;
}

void generator_c::add_instruction(const std::vector<uint8_t> instruction)
{
  _instruction_data.insert(_instruction_data.end(), _instruction_data.begin(),
                           _instruction_data.end());
}

std::vector<uint8_t> generator_c::generate_binary() const
{
  auto encoded_compatibility = pack_4(libskiff::binary::compatibility);
  auto encoded_number_of_constants = pack_8(_number_of_constants);
  std::vector<uint8_t> filler = {0xFF, 0xFF};

  std::vector<uint8_t> binary;
  binary.insert(binary.end(), encoded_compatibility.begin(),
                encoded_compatibility.end());
  binary.insert(binary.end(), encoded_number_of_constants.begin(),
                encoded_number_of_constants.end());
  binary.insert(binary.end(), filler.begin(), filler.end());
  binary.insert(binary.end(), _constant_data.begin(), _constant_data.end());
  binary.insert(binary.end(), filler.begin(), filler.end());
  binary.insert(binary.end(), _instruction_data.begin(),
                _instruction_data.end());
  return binary;
}

} // namespace binary
} // namespace libskiff
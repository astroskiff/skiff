#include "machine/memory/memory.hpp"
#include "config.hpp"
#include <cstring>

namespace skiff {
namespace machine {
namespace memory {

memory_c::memory_c(const uint64_t size) : _size(size), _data{nullptr}
{
  // Ensure that the memory is divisible by words (2 bytes)
  if (_size % 2 != 0) {
    _size++;
  }
  _data = new uint8_t[_size];
}

memory_c::~memory_c()
{
  if (_data) {
    delete[] _data;
  }
}

bool memory_c::put_hword(const uint64_t index, const uint8_t data)
{
  if (index >= _size) {
    return false;
  }
  _data[index] = data;
  return true;
}

bool memory_c::put_word(const uint64_t index, const uint16_t data)
{
  if (index + skiff::config::word_size_bytes >= _size) {
    return false;
  }
  _data[index] = data >> 8;
  _data[index + 1] = data;
  return true;
}

bool memory_c::put_dword(const uint64_t index, const uint32_t data)
{
  if (index + skiff::config::d_word_size_bytes >= _size) {
    return false;
  }
  _data[index] = data >> 24;
  _data[index + 1] = data >> 16;
  _data[index + 2] = data >> 8;
  _data[index + 3] = data;
  return true;
}

bool memory_c::put_qword(const uint64_t index, const uint64_t data)
{
  if (index + skiff::config::q_word_size_bytes >= _size) {
    return false;
  }
  _data[index] = data >> 56;
  _data[index + 1] = data >> 48;
  _data[index + 2] = data >> 40;
  _data[index + 3] = data >> 32;
  _data[index + 4] = data >> 24;
  _data[index + 5] = data >> 16;
  _data[index + 6] = data >> 8;
  _data[index + 7] = data;
  return true;
}

std::tuple<bool, uint8_t> memory_c::get_hword(const uint64_t index)
{
  if (index > _size) {
    return {false, 0};
  }
  return {true, _data[index]};
}

std::tuple<bool, uint16_t> memory_c::get_word(const uint64_t index)
{
  if (index + skiff::config::word_size_bytes > _size) {
    return {false, 0};
  }
  uint16_t d = static_cast<uint16_t>(_data[index]) << 8;
  d |= static_cast<uint16_t>(_data[index + 1]);
  return {true, d};
}

std::tuple<bool, uint32_t> memory_c::get_dword(const uint64_t index)
{
  if (index + skiff::config::d_word_size_bytes > _size) {
    return {false, 0};
  }

  uint32_t d = static_cast<uint32_t>(_data[index]) << 24;
  d |= static_cast<uint32_t>(_data[index + 1]) << 16;
  d |= static_cast<uint32_t>(_data[index + 2]) << 8;
  d |= static_cast<uint32_t>(_data[index + 3]);
  return {true, d};
}

std::tuple<bool, uint64_t> memory_c::get_qword(const uint64_t index)
{
  if (index + skiff::config::q_word_size_bytes > _size) {
    return {false, 0};
  }

  uint64_t d = static_cast<uint64_t>(_data[index]) << 56;
  d |= static_cast<uint64_t>(_data[index + 1]) << 48;
  d |= static_cast<uint64_t>(_data[index + 2]) << 40;
  d |= static_cast<uint64_t>(_data[index + 3]) << 32;
  d |= static_cast<uint64_t>(_data[index + 4]) << 24;
  d |= static_cast<uint64_t>(_data[index + 5]) << 16;
  d |= static_cast<uint64_t>(_data[index + 6]) << 8;
  d |= static_cast<uint64_t>(_data[index + 7]);
  return {true, d};
}

bool memory_c::import(const std::vector<uint8_t> &data)
{
  if (data.size() > _size) {
    return false;
  }
  std::memcpy(_data, &data[0], _size);
  return true;
}

} // namespace memory
} // namespace machine
} // namespace skiff
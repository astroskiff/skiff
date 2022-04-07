#include "libskiff/machine/stack.hpp"

#include "libskiff/system.hpp"

namespace libskiff {
namespace machine {

stack_c::stack_c() : _end{0}, _data{nullptr}
{
  _data = new uint8_t[libskiff::system::stack_size_bytes];
}

stack_c::~stack_c() { delete[] _data; }

std::tuple<bool, uint16_t> stack_c::pop_word()
{
  if (_end < libskiff::system::word_size_bytes) {
    return {false, 0};
  }
  uint16_t d = static_cast<uint16_t>(_data[--_end]);
  d |= static_cast<uint16_t>(_data[--_end]) << 8;
  return {true, d};
}

std::tuple<bool, uint32_t> stack_c::pop_dword()
{
  if (_end < libskiff::system::d_word_size_bytes) {
    return {false, 0};
  }

  uint32_t d = static_cast<uint32_t>(_data[--_end]);
  d |= static_cast<uint32_t>(_data[--_end]) << 8;
  d |= static_cast<uint32_t>(_data[--_end]) << 16;
  d |= static_cast<uint32_t>(_data[--_end]) << 24;
  return {true, d};
}

std::tuple<bool, uint64_t> stack_c::pop_qword()
{
  if (_end < libskiff::system::q_word_size_bytes) {
    return {false, 0};
  }
  uint64_t d = static_cast<uint64_t>(_data[--_end]);
  d |= static_cast<uint64_t>(_data[--_end]) << 8;
  d |= static_cast<uint64_t>(_data[--_end]) << 16;
  d |= static_cast<uint64_t>(_data[--_end]) << 24;
  d |= static_cast<uint64_t>(_data[--_end]) << 32;
  d |= static_cast<uint64_t>(_data[--_end]) << 40;
  d |= static_cast<uint64_t>(_data[--_end]) << 48;
  d |= static_cast<uint64_t>(_data[--_end]) << 56;
  return {true, d};
}

bool stack_c::push_word(const uint16_t word)
{
  if (_end + libskiff::system::word_size_bytes >
      libskiff::system::stack_size_bytes) {
    return false;
  }
  _data[_end++] = word >> 8;
  _data[_end++] = word;
  return true;
}

bool stack_c::push_dword(const uint32_t dword)
{
  if (_end + libskiff::system::word_size_bytes >
      libskiff::system::stack_size_bytes) {
    return false;
  }
  _data[_end++] = dword >> 24;
  _data[_end++] = dword >> 16;
  _data[_end++] = dword >> 8;
  _data[_end++] = dword;
  return true;
}

bool stack_c::push_qword(const uint64_t qword)
{
  if (_end + libskiff::system::word_size_bytes >
      libskiff::system::stack_size_bytes) {
    return false;
  }
  _data[_end++] = qword >> 56;
  _data[_end++] = qword >> 48;
  _data[_end++] = qword >> 40;
  _data[_end++] = qword >> 32;
  _data[_end++] = qword >> 24;
  _data[_end++] = qword >> 16;
  _data[_end++] = qword >> 8;
  _data[_end++] = qword;
  return true;
}

} // namespace machine
} // namespace libskiff
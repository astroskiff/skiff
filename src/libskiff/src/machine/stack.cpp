#include "libskiff/machine/stack.hpp"

#include "libskiff/system.hpp"

namespace libskiff {
namespace machine {

stack_c::stack_c() : _end{0}, _mem(libskiff::system::stack_size_bytes)
{
}

stack_c::~stack_c() { }

std::tuple<bool, uint16_t> stack_c::pop_word()
{
  if (_end < libskiff::system::word_size_bytes) {
    return {false, 0};
  }
  _end -= libskiff::system::word_size_bytes;
  return _mem.get_word(_end);
}

std::tuple<bool, uint32_t> stack_c::pop_dword()
{
  if (_end < libskiff::system::d_word_size_bytes) {
    return {false, 0};
  }
  _end -= libskiff::system::d_word_size_bytes;
  return _mem.get_dword(_end);
}

std::tuple<bool, uint64_t> stack_c::pop_qword()
{
  if (_end < libskiff::system::q_word_size_bytes) {
    return {false, 0};
  }
  _end -= libskiff::system::q_word_size_bytes;
  return _mem.get_qword(_end);
}

bool stack_c::push_word(const uint16_t word)
{
  if (_end + libskiff::system::word_size_bytes >=
      libskiff::system::stack_size_bytes) {
    return false;
  }
  _mem.put_word(_end, word);
  _end += libskiff::system::word_size_bytes;
  return true;
}

bool stack_c::push_dword(const uint32_t dword)
{
  if (_end + libskiff::system::d_word_size_bytes >=
      libskiff::system::stack_size_bytes) {
    return false;
  }
  _mem.put_dword(_end, dword);
  _end += libskiff::system::d_word_size_bytes;
  return true;
}

bool stack_c::push_qword(const uint64_t qword)
{
  if (_end + libskiff::system::q_word_size_bytes >=
      libskiff::system::stack_size_bytes) {
    return false;
  }
  _mem.put_qword(_end, qword);
  _end += libskiff::system::q_word_size_bytes;
  return true;
}

std::tuple<bool, uint16_t> stack_c::load_word(const uint64_t index)
{
  return _mem.get_word(index);
}

std::tuple<bool, uint32_t> stack_c::load_dword(const uint64_t index)
{
  return _mem.get_dword(index);
}

std::tuple<bool, uint64_t> stack_c::load_qword(const uint64_t index)
{
  return _mem.get_qword(index);
}

} // namespace machine
} // namespace libskiff
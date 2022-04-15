#ifndef LIBSKIFF_STACK_HPP
#define LIBSKIFF_STACK_HPP

#include "libskiff/machine/memory/memory.hpp"
#include "libskiff/types.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

namespace libskiff {
namespace machine {
namespace memory {

//! \brief A stack structure that contains up-to
//!        the number of bytes described by
//!        'stack_size_bytes' in libskiff/config.hpp
class stack_c {
public:
  //! \brief Create the stack
  stack_c();

  //! \brief Destroy the stack
  ~stack_c();

  //! \brief Set the stack pointer
  //! \post  If this is set, then the stack pointer register will
  //!        be auto updated whenever the stack size changes
  void set_sp(libskiff::types::vm_register &reg);

  //! \brief   Pop word
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  std::tuple<bool, uint16_t> pop_word();

  //! \brief   Pop double word
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  std::tuple<bool, uint32_t> pop_dword();

  //! \brief   Pop quad word
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  std::tuple<bool, uint64_t> pop_qword();

  //! \brief Push word
  //! \param word The word to push
  //! \returns true iff the operation was a success
  bool push_word(const uint16_t word);

  //! \brief Push double word
  //! \param word The word to push
  //! \returns true iff the operation was a success
  bool push_dword(const uint32_t dword);

  //! \brief Push quad word
  //! \param word The word to push
  //! \returns true iff the operation was a success
  bool push_qword(const uint64_t qword);

  //! \brief Store word
  //! \param destination The location in the stack to store the word
  //! \param value The value to store in memory at the location
  bool store_word(const uint64_t destination, const uint16_t value);

  //! \brief Store double word
  //! \param destination The location in the stack to store the word
  //! \param value The value to store in memory at the location
  bool store_dword(const uint64_t destination, const uint32_t value);

  //! \brief Store quad word
  //! \param destination The location in the stack to store the word
  //! \param value The value to store in memory at the location
  bool store_qword(const uint64_t destination, const uint64_t value);

  //! \brief Load word
  //! \param index The location to read the word from
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  std::tuple<bool, uint16_t> load_word(const uint64_t index);

  //! \brief Load double word
  //! \param index The location to read the word from
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  std::tuple<bool, uint32_t> load_dword(const uint64_t index);

  //! \brief Load quad word
  //! \param index The location to read the word from
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  std::tuple<bool, uint64_t> load_qword(const uint64_t index);

private:
  uint64_t _end;
  memory_c _mem;
  libskiff::types::vm_register *_sp;
};

} // namespace memory
} // namespace machine
} // namespace libskiff

#endif
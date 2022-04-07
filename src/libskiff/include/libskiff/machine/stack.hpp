#ifndef LIBSKIFF_STACK_HPP
#define LIBSKIFF_STACK_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>

namespace libskiff {
namespace machine {

//! \brief A stack structure that contains up-to
//!        the number of bytes described by
//!        'stack_size_bytes' in libskiff/system.hpp
class stack_c {
public:
  //! \brief Create the stack
  stack_c();

  //! \brief Destroy the stack
  ~stack_c();

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

private:
  uint64_t _end;
  uint8_t *_data;
};

} // namespace machine
} // namespace libskiff

#endif
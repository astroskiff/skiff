#ifndef LIBSKIFF_MEMORY_HPP
#define LIBSKIFF_MEMORY_HPP

#include <cstdint>
#include <tuple>

namespace libskiff {
namespace machine {

//! \brief A memory structure that contains up-to
//!        the number of bytes passed in at
//!        construction time
class memory_c {
public:
  //! \brief Create the memory
  memory_c(const uint64_t size);

  //! \brief Destroy the memory
  ~memory_c();

  //! \brief Store word
  //! \param destination The location in memory to store the word
  //! \param value The value to store in memory at the location
  bool put_word(const uint64_t index, const uint16_t data);

  //! \brief Store double word
  //! \param destination The location in memory to store the dword
  //! \param value The value to store in memory at the location
  bool put_dword(const uint64_t index, const uint32_t data);

  //! \brief Store quad word
  //! \param destination The location in memory to store the qword
  //! \param value The value to store in memory at the location
  bool put_qword(const uint64_t index, const uint64_t data);

  //! \brief Get word
  //! \param index The location to read the word from
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  std::tuple<bool, uint16_t> get_word(const uint64_t index);

  //! \brief Get double word
  //! \param index The location to read the word from
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  std::tuple<bool, uint32_t> get_dword(const uint64_t index);

  //! \brief Get quad word
  //! \param index The location to read the word from
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  std::tuple<bool, uint64_t> get_qword(const uint64_t index);

private:
  uint64_t _size;
  uint8_t *_data;
};

} // namespace machine
} // namespace libskiff

#endif
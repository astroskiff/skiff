#ifndef SKIFF_MEMORY_HPP
#define SKIFF_MEMORY_HPP

#include <cstdint>
#include <tuple>
#include <vector>

namespace skiff {
namespace machine {
namespace memory {

//! \brief A memory structure that contains up-to
//!        the number of bytes passed in at
//!        construction time
class memory_c {
public:
  //! \brief Create the memory
  memory_c(const uint64_t size);

  //! \brief Destroy the memory
  ~memory_c();

  //! \brief Store half word
  //! \param destination The location in memory to store the word
  //! \param value The value to store in memory at the location
  [[nodiscard]] bool put_hword(const uint64_t index, const uint8_t data);

  //! \brief Store word
  //! \param destination The location in memory to store the word
  //! \param value The value to store in memory at the location
  [[nodiscard]] bool put_word(const uint64_t index, const uint16_t data);

  //! \brief Store double word
  //! \param destination The location in memory to store the dword
  //! \param value The value to store in memory at the location
  [[nodiscard]] bool put_dword(const uint64_t index, const uint32_t data);

  //! \brief Store quad word
  //! \param destination The location in memory to store the qword
  //! \param value The value to store in memory at the location
  [[nodiscard]] bool put_qword(const uint64_t index, const uint64_t data);

  //! \brief Get half word
  //! \param index The location to read the half word from
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  [[nodiscard]] std::tuple<bool, uint8_t> get_hword(const uint64_t index);

  //! \brief Get word
  //! \param index The location to read the word from
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  [[nodiscard]] std::tuple<bool, uint16_t> get_word(const uint64_t index);

  //! \brief Get double word
  //! \param index The location to read the word from
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  [[nodiscard]] std::tuple<bool, uint32_t> get_dword(const uint64_t index);

  //! \brief Get quad word
  //! \param index The location to read the word from
  //! \returns tuple containing boolean indicating if
  //!          the operation was a success, and a value
  //!          from the stack.
  [[nodiscard]] std::tuple<bool, uint64_t> get_qword(const uint64_t index);

  //! \brief Retrieve the size of the data
  //! \returns Allocated size
  [[nodiscard]] uint64_t size() const { return _size; };

  //! \brief Retrieve 'n' bytes from memory
  //! \returns 'n' bytes from index of start iff range of [start, n] is valid
  [[nodiscard]] std::vector<uint8_t> get_n_bytes(const uint64_t start,
                                                 const uint64_t n);

  //! \brief Put bytes at position
  //! \returns true iff bytes will fit
  [[nodiscard]] bool put_n_bytes(const std::vector<uint8_t> data,
                                 const uint64_t start);

private:
  uint64_t _size;
  uint8_t *_data;
};

} // namespace memory
} // namespace machine
} // namespace skiff

#endif
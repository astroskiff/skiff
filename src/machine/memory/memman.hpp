#ifndef SKIFF_MEMMAN_HPP
#define SKIFF_MEMMAN_HPP

#include "machine/memory/memory.hpp"
#include <cstdint>
#include <mutex>
#include <optional>
#include <queue>
#include <tuple>
#include <vector>

namespace skiff {
namespace machine {
namespace memory {

//! \brief Memory manager class that allows the creation and
//!        retrieval of memory slots that can be used to hold information
class memman_c {
public:
  //! \brief Construct the memory manager
  memman_c();

  //! \brief Destruct the memory manager
  ~memman_c();

  //! \brief Allocate a memory slot of `size` bytes
  //! \param size The number of bytes to allocate
  //! \returns Tuple with a bool indicating if the allocation happened,
  //!          and a uint64_t that can be used to retrieve the slot later
  std::tuple<bool, uint64_t> alloc(const uint64_t size);

  //! \brief Free a slot
  //! \param id The id of the slot to free
  //! \returns true iff the slot existed and could be freed
  bool free(const uint64_t id);

  //! \brief Retrieve a slot
  //! \param id The id of the slot to retrive
  //! \returns Memory slot iff the id was valid, nullptr otherwise
  skiff::machine::memory::memory_c *get_slot(const uint64_t id);

private:
  std::vector<skiff::machine::memory::memory_c *> _slots;
  std::queue<std::size_t> _available_ids;
  std::mutex _mutex;
};

} // namespace memory
} // namespace machine
} // namespace skiff

#endif
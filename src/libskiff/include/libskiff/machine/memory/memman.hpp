#ifndef LIBSKIFF_MEMMAN_HPP
#define LIBSKIFF_MEMMAN_HPP

#include <tuple>
#include <optional>
#include <cstdint>
#include <queue>
#include <vector>
#include "libskiff/machine/memory.hpp"

namespace libskiff
{
namespace machine
{
namespace memory {

//! \brief Memory manager class that allows the creation and 
//!        retrieval of memory slots that can be used to hold information
class memman_c
{
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
  libskiff::machine::memory_c * get_slot(const uint64_t id) const;

private:
  std::vector<libskiff::machine::memory_c*> _slots;
  std::queue<std::size_t> _available_ids;
};

}
}
}

#endif
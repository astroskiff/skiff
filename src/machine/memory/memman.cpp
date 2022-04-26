#include "machine/memory/memman.hpp"

namespace skiff {
namespace machine {
namespace memory {

memman_c::memman_c() {}

memman_c::~memman_c()
{
  for (auto i = 0; i < _slots.size(); i++) {
    if (nullptr != _slots.at(i)) {
      delete _slots.at(i);
    }
  }
}

std::tuple<bool, uint64_t> memman_c::alloc(const uint64_t size)
{
  // Determine if the list needs to grow, or if there is an available index
  if (_available_ids.empty()) {
    _slots.push_back(new skiff::machine::memory::memory_c(size));
    return {true, _slots.size() - 1};
  }
  else {

    //  If there was a freed spot its index will be in the queue.
    //  Use it instead of growing the vector
    auto idx = _available_ids.front();
    _slots[idx] = new skiff::machine::memory::memory_c(size);
    _available_ids.pop();
    return {true, idx};
  }
}

bool memman_c::free(const uint64_t id)
{
  if (id >= _slots.size() || nullptr == _slots[id]) {
    return false;
  }
  delete _slots.at(id);
  _slots.at(id) = nullptr;
  _available_ids.push(id);
  return true;
}

skiff::machine::memory::memory_c *memman_c::get_slot(const uint64_t id) const
{
  if (id >= _slots.size() || nullptr == _slots[id]) {
    return nullptr;
  }
  return _slots.at(id);
}

} // namespace memory
} // namespace machine
} // namespace skiff
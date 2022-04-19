#ifndef LIBSKIFF_SYSTEM_INTERRUPT_EMITTER_HPP
#define LIBSKIFF_SYSTEM_INTERRUPT_EMITTER_HPP

#include "libskiff/machine/memory/memman.hpp"

namespace libskiff {
namespace machine {
namespace system {

//! \brief Something in the system that can emit an interrupt
class interrupt_emitter {
public:
  interrupt_emitter(libskiff::machine::memory::memman_c &vm_memory)
      : _protected_memman(vm_memory)
  {
  }
protected:
  libskiff::machine::memory::memman_c &_protected_memman;
};

} // namespace system
} // namespace machine
} // namespace libskiff

#endif
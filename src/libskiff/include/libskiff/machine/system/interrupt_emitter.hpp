#ifndef LIBSKIFF_SYSTEM_INTERRUPT_EMITTER_HPP
#define LIBSKIFF_SYSTEM_INTERRUPT_EMITTER_HPP

#include "libskiff/machine/memory/memman.hpp"
#include "libskiff/types.hpp"

namespace libskiff {
namespace machine {
namespace system {

//! \brief Something in the system that can emit an interrupt
class interrupt_emitter {
public:
  interrupt_emitter(libskiff::types::interrupt_cb cb,
                    libskiff::machine::memory::memman_c &vm_memory)
      : _protected_emit_interrupt(cb), _protected_memman(vm_memory)
  {
  }

protected:
  libskiff::types::interrupt_cb _protected_emit_interrupt;
  libskiff::machine::memory::memman_c &_protected_memman;
};

} // namespace system
} // namespace machine
} // namespace libskiff

#endif
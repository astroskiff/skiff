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
  //! \brief Construc the emitter
  //! \param cb The function pointer to the vm interrupt method
  //! \param vm_memory A reference to VM Memory so the emitter
  //!                  can ingest commands directly from memory,
  //!                  and respond using memory
  interrupt_emitter(libskiff::types::interrupt_cb cb,
                    libskiff::machine::memory::memman_c &vm_memory)
      : _protected_emit_interrupt(cb), _protected_memman(vm_memory)
  {
  }

protected:
  libskiff::types::interrupt_cb _protected_emit_interrupt;  //! The function pointer
  libskiff::machine::memory::memman_c &_protected_memman;   //! VM memory
};

} // namespace system
} // namespace machine
} // namespace libskiff

#endif
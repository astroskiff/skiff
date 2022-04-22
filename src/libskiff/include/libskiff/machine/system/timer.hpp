#ifndef LIBSKIFF_SYSTEM_TIMER_HPP
#define LIBSKIFF_SYSTEM_TIMER_HPP

#include "libskiff/machine/memory/memman.hpp"
#include "libskiff/machine/system/callable.hpp"
#include "libskiff/machine/system/interrupt_emitter.hpp"

namespace libskiff {
namespace machine {
namespace system {

//! \brief An object that can be used to setup multiple timers
class timer_c : public interrupt_emitter, public callable_if {
public:
  //! \brief Construct the timer object
  //! \param interrupt The interrupt function
  //! \param vm_memory The memory manager defined within the VM used
  //!        to communicate information back to the VM during an
  //!        interrupt
  //! \note  If the timer goes off while interrupts are disabled, the timer will
  //! attempt
  //!        to send the interrupt 10 times with 1ms between each retry. This
  //!        means that under certain circumstances a timer may be off by 10ms.
  timer_c(libskiff::types::interrupt_cb interrupt,
          libskiff::machine::memory::memman_c &vm_memory);

  //! \brief Destruct timer
  ~timer_c();

  //! \brief Spawn a timer thread
  //! vm_param: i0 - Time (ms) to run a timer
  //! vm_param: i1 - Interrupt id to send upon completion
  virtual void execute(libskiff::types::view_t &view) override;
};

} // namespace system
} // namespace machine
} // namespace libskiff

#endif
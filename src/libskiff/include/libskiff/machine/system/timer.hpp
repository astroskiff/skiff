#ifndef LIBSKIFF_SYSTEM_TIMER_HPP
#define LIBSKIFF_SYSTEM_TIMER_HPP

#include "libskiff/machine/memory/memman.hpp"
#include "libskiff/machine/system/callable.hpp"
#include "libskiff/machine/system/interrupt_emitter.hpp"

namespace libskiff {
namespace machine {
namespace system {

class timer_c : public interrupt_emitter, public callable_if {

public:
  timer_c(libskiff::machine::memory::memman_c &vm_memory);

  virtual void execute(libskiff::types::view_t &view) override;
};

} // namespace system
} // namespace machine
} // namespace libskiff

#endif
#ifndef LIBSKIFF_SYSTEM_SYSTEM_IF_HPP
#define LIBSKIFF_SYSTEM_SYSTEM_IF_HPP

#include "libskiff/machine/vm.hpp"

namespace libskiff {
namespace machine {
namespace system {

//! \brief An interface for callable items
class call_if {
public:
  virtual ~call_if() = default;

  //! \brief Method called when the item is triggered from a syscall
  //! \param view A view into the system state for int registers, float registers,
  //!             the op register, and the memory manager. Everything needed to
  //!             pass parameters to the callable and hand back data. 
  //!             - Convention is to use integer registers for parameter passing
  //!               and the op register to indicate success/failure. 
  //!               0 = failure, 1 = success
  virtual void execute(libskiff::machine::vm_c::view_t &view) = 0;
};

} // namespace system
} // namespace machine
} // namespace libskiff

#endif
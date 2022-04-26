/*
  Callable items are called via syscall and are meant to be a one-way
  short-lived call. Anything that requires a back and forth with the VM should
  be constructed as a device.
*/

#ifndef SKIFF_SYSTEM_CALLABLE_IF_HPP
#define SKIFF_SYSTEM_CALLABLE_IF_HPP

#include "types.hpp"

namespace skiff {
namespace machine {
namespace system {

//! \brief An interface for callable items
class callable_if {
public:
  virtual ~callable_if() = default;

  //! \brief Method called when the item is triggered from a syscall
  //! \param view A view into the system state for int registers, float
  //! registers,
  //!             the op register, and the memory manager. Everything needed to
  //!             pass parameters to the callable and hand back data.
  //!             - Convention is to use integer registers for parameter passing
  //!               and the op register to indicate success/failure.
  //!               0 = failure, 1 = success
  virtual void execute(skiff::types::view_t &view) = 0;
};

} // namespace system
} // namespace machine
} // namespace skiff

#endif
#ifndef SKIFF_SYSTEM_PRINT_HPP
#define SKIFF_SYSTEM_PRINT_HPP

#include "machine/system/callable.hpp"

namespace skiff {
namespace machine {
namespace system {

//! \brief An interface to print information from memory
class print_c : public callable_if {
public:
  //! \brief Prints data to the screen
  //! vm_param: i0 - Memory slot containing data
  //! vm_param: i1 - Offset into memory slot data starts at
  //! vm_param: i2 - Length (in bytes) to print
  //! vm_param: i3 - Data type
  //!                0 - u8,  1 - i8,  2 - u16,  3 - i16
  //!                4 - u32, 5 - i32, 6 - u64,  7 - i64
  //!                8 - float, 9 - ASCII
  //! vm_param: i4 - If 0 is not present in i4, a new line will print
  //! vm_retval: Failure to print will result in the op register
  //!            being set to a value of `0` and `1` on success
  //! \note The vm_param in i2 is used to ensure there is no data
  //!       overruns in memory and to obtain strings. This means that
  //!       stating a length of 6 bytes while trying to print a `u8`
  //!       will only result in the `u8` being printed (iff there are
  //!       6 bytes of space from offset->length free) and not a full
  //!       6 bytes.
  virtual void execute(skiff::types::view_t &view) override;
};

} // namespace system
} // namespace machine
} // namespace skiff

#endif
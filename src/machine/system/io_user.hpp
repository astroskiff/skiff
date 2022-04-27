#ifndef SKIFF_SYSTEM_IO_USER
#define SKIFF_SYSTEM_IO_USER

#include "machine/system/callable.hpp"

namespace skiff {
namespace machine {
namespace system {

/*

Memory Slot Layout

Iff command is a success `op` register will be set to `1`

WORD: [Command]

Command 0: (output)
  WORD: [Memory slot containing data]
  WORD: [Offset into slot data starts at]
  WORD: [Data type]
          0 - u8,  1 - i8,  2 - u16,  3 - i16
          4 - u32, 5 - i32, 6 - u64,  7 - i64
          8 - float, 9 - ASCII
  WORD: [Length (chars) to print (applies to string only)]
  WORD: [Send to stdout (bool)]
  WORD: [Print newline  (bool)]

  Sets op register to `1` on success

Command 1: (input)
  WORD: [Memory slot to place data]
  WORD: [Offset into slot to start placing data]
  WORD: [Data type]
          0 - u8,  1 - i8,  2 - u16,  3 - i16
          4 - u32, 5 - i32, 6 - u64,  7 - i64
          8 - float, 9 - ASCII
  WORD: [Length (chars) to read (applies to string only)]

  Sets op register to length read in iff a string was read in on success,
  otherwise it sets op register to `1` in success
*/

//! \brief An interface to to i/o with a user
class io_user_c : public callable_if {
public:
  //! \brief Performs User I/O Operation
  //! vm_param: i0 - Slot to read in command from
  //! vm_param: i1 - Offset within slot to command
  //! vm_retval: Failure to perform requested iperation result
  //!            in the op register being set to a value of `0`
  //!            The value on success is dependant on the command 
  virtual void execute(skiff::types::view_t &view) override;

private:
  void perform_output(skiff::types::view_t &view, const uint16_t source,
                      const uint16_t offset, const uint16_t data_type,
                      const uint16_t length, const uint16_t destination,
                      const uint16_t newline);
  void perform_input(skiff::types::view_t &view, const uint16_t destination,
                     const uint16_t offset, const uint16_t data_type,
                     const uint16_t length);
};

} // namespace system
} // namespace machine
} // namespace skiff

#endif
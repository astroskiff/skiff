#ifndef SKIFF_SYSTEM_IO_DISK
#define SKIFF_SYSTEM_IO_DISK

#include "machine/system/callable.hpp"
#include <memory>

namespace skiff {
namespace machine {
namespace system {

/*

Command

WORD [Create = 0 | Load = 1 | Close = 2 ]

Create
  QWORD [Filename Length]
  


*/

// fwd 
class file_manager_c;

//! \brief An interface to to i/o with a user
class io_disk_c : public callable_if {
public:
  io_disk_c();
  ~io_disk_c();

  //! \brief Performs Disk I/O Operation
  //! vm_param: i0 - Slot to read in command from
  //! vm_param: i1 - Offset within slot to command
  //! vm_retval: Failure to perform requested iperation result
  //!            in the op register being set to a value of `0`
  //!            The value on success is dependant on the command
  virtual void execute(skiff::types::view_t &view) override;

private:
  file_manager_c* _manager{nullptr};  // Use raw ptr bc opaque type 
};

} // namespace system
} // namespace machine
} // namespace skiff

#endif
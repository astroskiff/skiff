#ifndef SKIFF_SYSTEM_IO_DISK
#define SKIFF_SYSTEM_IO_DISK

#include "machine/memory/memory.hpp"
#include "machine/system/callable.hpp"

namespace skiff {
namespace machine {
namespace system {

/*

Command

WORD [Create = 0 | Open = 1 | Close = 2 | Write = 3 | Read = 4]

Create
  QWORD [File path source slot]
  QWORD [File path source slot offset]
  QWORD [File path length]

  Results with op register set to `1` if success, 0 otherwise

  Sets i0 to the ID for the file descriptor

Open
  QWORD [File Descriptor]
  WORD [Flags]
    app    = 0x01;
    ate    = 0x02;
    binary = 0x04;
    in     = 0x08;
    out    = 0x10;
    trunc  = 0x20;

  Results with op register set to `1` if success, 0 otherwise

Close
  QWORD [File Descriptor]

  Results with op register set to `1` if success, 0 otherwise

Write
  QWORD [File Descriptor]
  QWORD [Source slot]
  QWORD [Source slot offset]
  QWORD [Number of bytes]

  Results with op register set to `1` if success, 0 otherwise

Read
  QWORD [File Descriptor]
  QWORD [Destination slot]
  QWORD [Destination slot offset]
  QWORD [Number of bytes]

  Results with op register to the number of bytes read in
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
  file_manager_c *_manager{nullptr};
  void create(skiff::machine::memory::memory_c *slot,
              skiff::types::view_t &view);
  void open(skiff::machine::memory::memory_c *slot, skiff::types::view_t &view);
  void close(skiff::machine::memory::memory_c *slot,
             skiff::types::view_t &view);
  void write(skiff::machine::memory::memory_c *slot,
             skiff::types::view_t &view);
  void read(skiff::machine::memory::memory_c *slot, skiff::types::view_t &view);
};

} // namespace system
} // namespace machine
} // namespace skiff

#endif
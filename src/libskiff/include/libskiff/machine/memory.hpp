#ifndef LIBSKIFF_MEMORY_HPP
#define LIBSKIFF_MEMORY_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

namespace libskiff {
namespace machine {

/*

  _________________________________
  |                                |
  |     Program Stack              |
  |                                |
  |________________________________|
          /|\
            |________ Grows and shrinks as-per instructed

  For 'heap' memory, an instruction must executed

  mreq i0 @1024 ; Request 1024 bytes of memory, and store the address in i0

  mov i3 @21
  mov i2 @0
  sw  i0 i2 i3   ; Store word from i3 into i0 at offset i2 - Pulls lowest 2 bytes
  sdw i0 i2 i3   ; Store double word - lowest 4 bytes
  sqw i0 i2 i3   ; Store quad word - 8 bytes

  <memory slot> <offset> <data>

  lw i3 i0 i2   ; <destination> <memory slot> <offset>
  ...
  ..
  ..

*/


//! \brief A memory structure for the nabla vsys
class memory_c {
public:
  memory_c();
};

} // namespace machine
} // namespace libskiff

#endif
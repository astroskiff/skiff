#include "libskiff/machine/system/timer.hpp"

namespace libskiff {
namespace machine {
namespace system {

timer_c::timer_c(libskiff::machine::memory::memman_c &vm_memory)
    : interrupt_emitter(vm_memory)
{
}

void timer_c::execute(libskiff::types::view_t &view)
{

}

} // namespace system
} // namespace machine
} // namespace libskiff
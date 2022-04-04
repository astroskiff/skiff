#include "libskiff/machine/vm.hpp"

/*

  Design thoughts:

    Should we pre-decode all of the instructions and store them in memory?
    It would save time at run-time but it might be a lot of overhead??


*/

namespace libskiff {
namespace machine {

vm_c::vm_c(std::unique_ptr<libskiff::binary::executable_c> executable)
{
  // Using the executable, we need to build instructions that are listed
  // in the execution_context



}

void vm_c::set_runtime_callback(libskiff::types::runtime_error_cb cb)
{
  _runtime_error_cb = {cb};
}

std::pair<vm_c::execution_result_e, int> vm_c::execute() 
{
  // Setup instructions

  // Create memory module

  // Create the original execution context

  // Kick off the runner

  return{execution_result_e::OKAY, 0};
}

} // namespace machine
} // namespace libskiff
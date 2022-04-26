#include "libskiff/machine/system/timer.hpp"
#include "libskiff/config.hpp"
#include "libskiff/logging/aixlog.hpp"

#include <chrono>
#include <iostream>

#ifdef LIBSKIFF_USE_THREADS
#include <thread>
#endif

namespace libskiff {
namespace machine {
namespace system {

#ifdef LIBSKIFF_USE_THREADS
namespace {
void timer_thread(std::function<bool(const uint64_t)> interrupt,
                  const uint64_t time_ms, const uint64_t interrupt_id)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(time_ms));

  uint8_t retry = 10;
  // its possible that the thread outlives the interrupt function call
  // even though unlikely, we wrap the whole cat in a try/catch
  try {
    while (retry-- > 0 && !interrupt(interrupt_id)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  catch (std::exception &e) {
    LOG(WARNING) << TAG("timer") << std::this_thread::get_id()
                 << " got exception: " << e.what() << "\n";
  }
}
} // namespace
#endif

timer_c::timer_c(std::function<bool(const uint64_t)> interrupt,
                 skiff::machine::memory::memman_c &vm_memory)
    : interrupt_emitter(interrupt, vm_memory)
{
}

timer_c::~timer_c() {}

void timer_c::execute(libskiff::types::view_t &view)
{
  view.op_register = 0;
#ifdef LIBSKIFF_USE_THREADS
  if (view.integer_registers[0] == 0) {
    return;
  }

  auto time_ms = view.integer_registers[0];
  auto interrupt_id = view.integer_registers[1];

  std::thread t(timer_thread, _protected_emit_interrupt, time_ms, interrupt_id);
  t.detach();

  view.op_register = 1;
#else 
  LOG(FATAL) << TAG("system:timer") << "System timer requires compile-time definition `LIBSKIFF_USE_THREADS` to function\n";
#endif
}

} // namespace system
} // namespace machine
} // namespace libskiff
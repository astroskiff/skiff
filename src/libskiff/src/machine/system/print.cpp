#include "libskiff/machine/system/print.hpp"
#include "libskiff/bytecode/floating_point.hpp"
#include <bitset>
#include <iostream>

namespace libskiff {
namespace machine {
namespace system {

namespace {
enum class data_t {
  U8 = 0,
  I8 = 1,
  U16 = 2,
  I16 = 3,
  U32 = 4,
  I32 = 5,
  U64 = 6,
  I64 = 7,
  FLOAT = 8,
  ASCII = 9
};
}

void print_c::execute(libskiff::types::view_t &view)
{
  /*
    std::cout <<
      "slot   : " << view.integer_registers[0] << "\n" <<
      "offset : " << view.integer_registers[1] << "\n" <<
      "length : " << view.integer_registers[2] << "\n" <<
      "type   : " << view.integer_registers[3] << "\n";
  */

  // Assume failure
  view.op_register = 0;

  // Get the memory slot
  // Ensure that the index is within the slot
  auto slot = view.memory_manager.get_slot(view.integer_registers[0]);
  if (!slot) {
    return;
  }

  // Ensure length wont overrun it
  auto data_offset = view.integer_registers[1];
  auto data_length = view.integer_registers[2];

  // Size returns byts, we need to calculate it using words,
  // so multiply data_length by 2
  if (slot->size() < data_offset + data_length) {
    return;
  }

  switch (static_cast<data_t>(view.integer_registers[3])) {
  case data_t::U8: {
    auto [okay, data] = slot->get_word(data_offset);
    if (!okay) {
      return;
    }
    std::cout << std::dec << data;
    break;
  }
  case data_t::I8: {
    auto [okay, data] = slot->get_word(data_offset);
    if (!okay) {
      return;
    }
    // This is dumb but it ensures that negative values print correctly
    std::cout << std::dec << static_cast<int>(static_cast<int8_t>(data));
    break;
  }
  case data_t::U16: {
    auto [okay, data] = slot->get_word(data_offset);
    if (!okay) {
      return;
    }
    std::cout << data;
    break;
  }
  case data_t::I16: {
    auto [okay, data] = slot->get_word(data_offset);
    if (!okay) {
      return;
    }
    std::cout << static_cast<int16_t>(data);
    break;
  }
  case data_t::U32: {
    auto [okay, data] = slot->get_dword(data_offset);
    if (!okay) {
      return;
    }
    std::cout << data;
    break;
  }
  case data_t::I32: {
    auto [okay, data] = slot->get_dword(data_offset);
    if (!okay) {
      return;
    }
    std::cout << static_cast<int32_t>(data);
    break;
  }
  case data_t::U64: {
    auto [okay, data] = slot->get_qword(data_offset);
    if (!okay) {
      return;
    }
    std::cout << data;
    break;
  }
  case data_t::I64: {
    auto [okay, data] = slot->get_qword(data_offset);
    if (!okay) {
      return;
    }
    std::cout << static_cast<int64_t>(data);
    break;
  }
  case data_t::FLOAT: {
    auto [okay, data] = slot->get_qword(data_offset);
    if (!okay) {
      return;
    }
    std::cout << libskiff::bytecode::floating_point::from_uint64_t(data);
    break;
  }
  case data_t::ASCII: {
    std::string out;
    decltype(data_length) idx = 0;
    decltype(data_length) num = 0;
    while (num++ != data_length) {
      auto [okay, c_word] = slot->get_word(idx);
      idx += 2;
      if (!okay) {
        break;
      }
      out += static_cast<char>(c_word);
    }
    std::cout << out;
    break;
  }
  default:
    return;
  };

  // Check for newline out
  if (view.integer_registers[4] != 0) {
    std::cout << std::endl;
  }

  // Indicate success
  view.op_register = 1;
}

} // namespace system
} // namespace machine
} // namespace libskiff
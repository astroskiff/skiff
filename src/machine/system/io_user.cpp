#include "machine/system/io_user.hpp"
#include "config.hpp"
#include <bitset>
#include <iostream>
#include <libskiff/bytecode/floating_point.hpp>

namespace skiff {
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

void io_user_c::execute(skiff::types::view_t &view)
{
  std::cout << "Called\n";

  // Assume failure
  view.op_register = 0;

  // Get the memory slot
  // Ensure that the index is within the slot
  auto slot = view.memory_manager.get_slot(view.integer_registers[0]);
  if (!slot) {
    return;
  }

  auto command_offset = view.integer_registers[1];

  auto [a, command] = slot->get_word(command_offset);
  if (!a) {
    return;
  }

  command_offset += skiff::config::word_size_bytes;
  auto [b, target_slot] = slot->get_word(command_offset);
  if (!b) {
    return;
  }

  command_offset += skiff::config::word_size_bytes;
  auto [c, target_offset] = slot->get_word(command_offset);
  if (!c) {
    return;
  }

  command_offset += skiff::config::word_size_bytes;
  auto [d, data_type] = slot->get_word(command_offset);
  if (!d) {
    return;
  }

  command_offset += skiff::config::word_size_bytes;
  auto [e, length] = slot->get_word(command_offset);
  if (!e) {
    return;
  }

  if (command == 0) {
    command_offset += skiff::config::word_size_bytes;
    auto [f, destination] = slot->get_word(command_offset);
    if (!f) {
      return;
    }
    command_offset += skiff::config::word_size_bytes;
    auto [g, newline] = slot->get_word(command_offset);
    if (!g) {
      return;
    }
    return perform_output(view, target_slot, target_offset, data_type, length,
                          destination, newline);
  }
  if (command == 1) {
    return perform_input(view, target_slot, target_offset, data_type, length);
  }
}

void io_user_c::perform_output(skiff::types::view_t &view,
                               const uint16_t source, const uint16_t offset,
                               const uint16_t data_type, const uint16_t length,
                               const uint16_t destination,
                               const uint16_t newline)
{
  auto slot = view.memory_manager.get_slot(source);
  if (!slot) {
    return;
  }

  if (slot->size() < offset) {
    return;
  }

  switch (static_cast<data_t>(data_type)) {
  case data_t::U8: {
    auto [okay, data] = slot->get_word(offset);
    if (!okay) {
      return;
    }
    std::cout << std::dec << data;
    break;
  }
  case data_t::I8: {
    auto [okay, data] = slot->get_word(offset);
    if (!okay) {
      return;
    }
    // This is dumb but it ensures that negative values print correctly
    std::cout << std::dec << static_cast<int>(static_cast<int8_t>(data));
    break;
  }
  case data_t::U16: {
    auto [okay, data] = slot->get_word(offset);
    if (!okay) {
      return;
    }
    std::cout << data;
    break;
  }
  case data_t::I16: {
    auto [okay, data] = slot->get_word(offset);
    if (!okay) {
      return;
    }
    std::cout << static_cast<int16_t>(data);
    break;
  }
  case data_t::U32: {
    auto [okay, data] = slot->get_dword(offset);
    if (!okay) {
      return;
    }
    std::cout << data;
    break;
  }
  case data_t::I32: {
    auto [okay, data] = slot->get_dword(offset);
    if (!okay) {
      return;
    }
    std::cout << static_cast<int32_t>(data);
    break;
  }
  case data_t::U64: {
    auto [okay, data] = slot->get_qword(offset);
    if (!okay) {
      return;
    }
    std::cout << data;
    break;
  }
  case data_t::I64: {
    auto [okay, data] = slot->get_qword(offset);
    if (!okay) {
      return;
    }
    std::cout << static_cast<int64_t>(data);
    break;
  }
  case data_t::FLOAT: {
    auto [okay, data] = slot->get_qword(offset);
    if (!okay) {
      return;
    }
    std::cout << libskiff::bytecode::floating_point::from_uint64_t(data);
    break;
  }
  case data_t::ASCII: {
    std::string out;
    auto num = offset;
    while (num != length + offset) {
      auto [okay, c_word] = slot->get_word(num++);
      if (!okay) {
        break;
      }
      out += static_cast<char>(c_word >> 8);
    }
    if(destination == 1) {
      std::cout << out;
    } else {
      std::cerr << out;
    }
    if (newline) {
      std::cout << std::endl;
    }
    break;
  }
  default:
    return;
  };




  view.op_register = 1;
}

void io_user_c::perform_input(skiff::types::view_t &view, const uint16_t source,
                              const uint16_t offset, const uint16_t data_type,
                              const uint16_t length)
{
  std::cout << "INPUT\n";
  std::cout << "source: " << source << ", offset: " << offset
            << ", type: " << data_type << ", len: " << length
            << std::endl;
  view.op_register = 1;
}

} // namespace system
} // namespace machine
} // namespace skiff
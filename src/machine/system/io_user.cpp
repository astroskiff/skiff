#include "machine/system/io_user.hpp"
#include "config.hpp"
#include <algorithm>
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

template <class T>
void output_data(T data, const uint16_t use_std_out, const uint16_t do_newline)
{
  std::string end = (do_newline == 1) ? "\n" : "";
  if (use_std_out == 1) {
    std::cout << data << end;
  }
  else {
    std::cerr << data << end;
  }
}
} // namespace

void io_user_c::execute(skiff::types::view_t &view)
{
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
    output_data<uint16_t>(data, destination, newline);
    break;
  }
  case data_t::I8: {
    auto [okay, data] = slot->get_word(offset);
    if (!okay) {
      return;
    }
    // This is dumb but it ensures that negative values print correctly
    output_data<int>(static_cast<int>(static_cast<int8_t>(data)), destination,
                     newline);
    break;
  }
  case data_t::U16: {
    auto [okay, data] = slot->get_word(offset);
    if (!okay) {
      return;
    }
    output_data<uint16_t>(data, destination, newline);
    break;
  }
  case data_t::I16: {
    auto [okay, data] = slot->get_word(offset);
    if (!okay) {
      return;
    }
    output_data<int16_t>(data, destination, newline);
    break;
  }
  case data_t::U32: {
    auto [okay, data] = slot->get_dword(offset);
    if (!okay) {
      return;
    }
    output_data<uint32_t>(data, destination, newline);
    break;
  }
  case data_t::I32: {
    auto [okay, data] = slot->get_dword(offset);
    if (!okay) {
      return;
    }
    output_data<int32_t>(static_cast<int32_t>(data), destination, newline);
    break;
  }
  case data_t::U64: {
    auto [okay, data] = slot->get_qword(offset);
    if (!okay) {
      return;
    }
    output_data<uint64_t>(data, destination, newline);
    break;
  }
  case data_t::I64: {
    auto [okay, data] = slot->get_qword(offset);
    if (!okay) {
      return;
    }
    output_data<int64_t>(static_cast<int64_t>(data), destination, newline);
    break;
  }
  case data_t::FLOAT: {
    auto [okay, data] = slot->get_qword(offset);
    if (!okay) {
      return;
    }
    output_data<double>(libskiff::bytecode::floating_point::from_uint64_t(data),
                        destination, newline);
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
    output_data<std::string>(out, destination, newline);
    break;
  }
  default:
    return;
  };
  view.op_register = 1;
}

void io_user_c::perform_input(skiff::types::view_t &view,
                              const uint16_t destination, const uint16_t offset,
                              const uint16_t data_type, const uint16_t length)
{
  auto slot = view.memory_manager.get_slot(destination);
  if (!slot) {
    return;
  }

  if (slot->size() < offset + length) {
    return;
  }

  switch (static_cast<data_t>(data_type)) {
  case data_t::U16:
    [[fallthrough]];
  case data_t::I8:
    [[fallthrough]];
  case data_t::I16:
  case data_t::U8: {
    uint16_t value = 0;
    std::cin >> value;
    if (!slot->put_word(offset, value)) {
      return;
    }
    break;
  }
  case data_t::I32:
    [[fallthrough]];
  case data_t::U32: {
    uint32_t value = 0;
    std::cin >> value;
    if (!slot->put_dword(offset, value)) {
      return;
    }
    break;
  }
  case data_t::U64:
    [[fallthrough]];
  case data_t::I64: {
    uint64_t value = 0;
    std::cin >> value;
    if (!slot->put_qword(offset, value)) {
      return;
    }
    break;
  }
  case data_t::FLOAT: {
    double value = 0.00;
    std::cin >> value;
    if (!slot->put_qword(
            offset, libskiff::bytecode::floating_point::to_uint64_t(value))) {
      return;
    }
    break;
  }
  case data_t::ASCII: {
    std::string value;
    std::getline(std::cin, value);
    auto actual_length =
        std::min(static_cast<std::size_t>(length), value.size());
    for (auto i = 0; i < actual_length; i++) {
      if (!slot->put_word(i, static_cast<uint16_t>(value[i]) << 8)) {
        return;
      }
    }
    view.op_register = actual_length;
    return;
  }
  default:
    return;
  };
  view.op_register = 1;
}

} // namespace system
} // namespace machine
} // namespace skiff
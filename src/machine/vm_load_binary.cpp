#include "defines.hpp"
#include "logging/aixlog.hpp"
#include "machine/vm.hpp"
#include "types.hpp"
#include <libskiff/bytecode/instructions.hpp>
#include <libskiff/types.hpp>
#include <libskiff/version.hpp>

namespace skiff {
namespace machine {

/*
    Load the binary into a series of objects that can visit the VM for
   execution. Pre-decoding the instructions this way saves us the time of
   splitting them up at execution time and lets us pre-check the binary for any
   illegal instructions before execution.

    These structures store references to registers as well meaning we won't need
    to determine where to place data at run time.
*/
bool vm_c::load(std::unique_ptr<libskiff::bytecode::executable_c> executable)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  // Check if its experimental
  if (executable->is_experimental()) {
    issue_forced_warning("Code marked experimental");
  }

  // Check compatibilty
  auto version = executable->get_compatiblity_semver();

  LOG(DEBUG) << TAG("vm") << "semver.major:"
             << (int)libskiff::version::semantic_version.major
             << "thisversion.major:  " << (int)version.major << "\n";

  if (libskiff::version::semantic_version.major < version.major) {
    issue_forced_error("Incompatibility detected : "
                       "Bytecode version.major newer than VM version.major. ");
    return false;
  }
  if (libskiff::version::semantic_version.minor < version.minor) {

    LOG(DEBUG) << TAG("vm") << "semver.minor:"
               << (int)libskiff::version::semantic_version.minor
               << "thisversion.minor:  " << (int)version.minor << "\n";

    issue_forced_warning(
        "Potential incompatibility detected : "
        "Bytecode version.minor newer than VM version.minor. ");
  }
  if (libskiff::version::semantic_version.patch < version.patch) {
    issue_forced_warning(
        "Potential incompatibility detected : "
        "Bytecode version.patch newer than VM version.patch. ");
  }

  // Set debug level
  _debug_level = executable->get_debug_level();

  // Set instruction pointer to the entry address
  _ip = executable->get_entry_address();

  // Grap interrupt table
  _interrupt_id_to_address = executable->get_interrupt_table();

  // Load constants
  {
    auto constant_bytes = executable->get_constants();
    if (!constant_bytes.empty()) {
      auto [okay, id] = _memman.alloc(constant_bytes.size());
      if (!okay) {
        std::string msg = "Unable to allocate [" +
                          std::to_string(constant_bytes.size()) +
                          "] bytes for constants";
        issue_forced_error(msg);
        return false;
      }
      auto memory_slot = _memman.get_slot(id);
      if (!memory_slot) {
        std::string msg = "Unable to retrieve memory id [" +
                          std::to_string(id) + "] bytes for constants";
        issue_forced_error(msg);
        return false;
      }

      if (!memory_slot->put_n_bytes(constant_bytes, 0)) {
        issue_forced_error("Unable to put constants into memory slot");
        return false;
      }
    }
  }

  /*
      Helper lambdas
  */

  auto decode_qword =
      [=](std::vector<uint8_t> &data) -> std::tuple<bool, uint64_t> {
    if (data.size() != 8) {
      return {false, 0};
    }
    uint8_t shift = 56;
    uint64_t value{0x00};
    for (auto i = 0; i < data.size(); i++) {
      value |= (static_cast<uint64_t>(data[i]) << shift);
      shift -= 8;
    }
    return {true, value};
  };

  auto decode_ins_with_one_reg = [&, this](std::vector<uint8_t> &data)
      -> std::tuple<bool, skiff::types::vm_register *> {
    if (data.size() != 1) {
      LOG(FATAL) << TAG("vm") << "Insufficent data to construct instruction\n";
      return {false, nullptr};
    }
    auto targeted_register = get_register(data[0]);
    if (!targeted_register) {
      LOG(FATAL) << TAG("vm") << "Unable to locate register by value\n";
      return {false, nullptr};
    }
    return {true, targeted_register};
  };

  auto decode_ins_with_two_reg = [&, this](std::vector<uint8_t> &data)
      -> std::tuple<bool, skiff::types::vm_register *,
                    skiff::types::vm_register *> {
    if (data.size() != 2) {
      LOG(FATAL) << TAG("vm") << "Insufficent data to construct instruction\n";
      return {false, nullptr, nullptr};
    }

    auto lhs = get_register(data[0]);
    if (!lhs) {
      LOG(FATAL) << TAG("vm") << "Unable to locate register by value\n";
      return {false, nullptr, nullptr};
    }

    auto rhs = get_register(data[1]);
    if (!rhs) {
      LOG(FATAL) << TAG("vm") << "Unable to locate register by value\n";
      return {false, nullptr, nullptr};
    }

    return {true, lhs, rhs};
  };

  auto decode_ins_with_three_reg = [&, this](std::vector<uint8_t> &data)
      -> std::tuple<bool, skiff::types::vm_register *,
                    skiff::types::vm_register *, skiff::types::vm_register *> {
    if (data.size() != 3) {
      LOG(FATAL) << TAG("vm") << "Insufficent data to construct instruction\n";
      return {false, nullptr, nullptr, nullptr};
    }

    auto one = get_register(data[0]);
    if (!one) {
      LOG(FATAL) << TAG("vm") << "Unable to locate register by value\n";
      return {false, nullptr, nullptr, nullptr};
    }

    auto two = get_register(data[1]);
    if (!two) {
      LOG(FATAL) << TAG("vm") << "Unable to locate register by value\n";
      return {false, nullptr, nullptr, nullptr};
    }

    auto three = get_register(data[2]);
    if (!three) {
      LOG(FATAL) << TAG("vm") << "Unable to locate register by value\n";
      return {false, nullptr, nullptr, nullptr};
    }

    return {true, one, two, three};
  };

  auto decode_branch_instruction = [&, this](std::vector<uint8_t> &data)
      -> std::tuple<bool, skiff::types::vm_register *,
                    skiff::types::vm_register *, uint64_t> {
    if (data.size() != 10) {
      return {false, nullptr, nullptr, 0};
    }

    auto lhs = get_register(data[0]);
    if (!lhs) {
      LOG(FATAL) << TAG("vm") << "Unable to locate register by value\n";
      return {false, nullptr, nullptr, 0};
    }

    auto rhs = get_register(data[1]);
    if (!rhs) {
      LOG(FATAL) << TAG("vm") << "Unable to locate register by value\n";
      return {false, nullptr, nullptr, 0};
    }

    std::vector<uint8_t> value = {data.begin() + 2, data.end()};
    auto [success, branch_destination] = decode_qword(value);
    if (!success) {
      LOG(FATAL) << TAG("vm") << "Unable to obtain branch destination\n";
      return {false, nullptr, nullptr, 0};
    }

    return {true, lhs, rhs, branch_destination};
  };

  // Create instructions - return false if illegal instruction found
  auto instructions = executable->get_instructions();
  auto instruction_size_map =
      libskiff::bytecode::instructions::get_instruction_to_size_map();
  for (std::size_t i = 0; i < instructions.size(); /* no op */) {

    auto opcode = instructions[i++];
    if (instruction_size_map.find(opcode) == instruction_size_map.end()) {
      LOG(FATAL) << TAG("vm")
                 << "Unknown instruction id: " << static_cast<int>(opcode)
                 << "\n";
      return false;
    }

    // We subtract one because we've already consumed the opcode
    auto instruction_length = instruction_size_map[opcode] - 1;

    // Check to ensure we have that number bytes left
    if (i + instruction_length > instructions.size()) {
      LOG(FATAL) << TAG("vm") << "Incomplete instruction at " << i << "/"
                 << instructions.size() << " trying to read "
                 << instruction_length << " bytes"
                 << "\n";
      return false;
    }

    // Retrieve the instruction data from space after opcode going the length of
    // the instruction
    std::vector<uint8_t> instruction_data = {instructions.begin() + i,
                                             instructions.begin() + i +
                                                 instruction_length};

    // inc i the length of the instruction
    i += instruction_length;

    switch (opcode) {
    case libskiff::bytecode::instructions::NOP: {
      LOG(DEBUG) << TAG("vm") << "Decoded `NOP`\n";
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_nop_c>());
      break;
    }
    case libskiff::bytecode::instructions::EXIT: {
      LOG(DEBUG) << TAG("vm") << "Decoded `EXIT`\n";
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_exit_c>());
      break;
    }
    case libskiff::bytecode::instructions::RET: {
      LOG(DEBUG) << TAG("vm") << "Decoded `RET`\n";
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_ret_c>());
      break;
    }
    case libskiff::bytecode::instructions::EIRQ: {
      LOG(DEBUG) << TAG("vm") << "Decoded `EIRQ`\n";
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_eirq_c>());
      break;
    }
    case libskiff::bytecode::instructions::DIRQ: {
      LOG(DEBUG) << TAG("vm") << "Decoded `DIRQ`\n";
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_dirq_c>());
      break;
    }
    case libskiff::bytecode::instructions::CALL: {
      LOG(DEBUG) << TAG("vm") << "Decoded `CALL`\n";
      auto [success, value] = decode_qword(instruction_data);
      if (!success) {
        LOG(FATAL) << TAG("vm") << "Failed to decode QWORD\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_call_c>(value));
      break;
    }
    case libskiff::bytecode::instructions::JMP: {
      LOG(DEBUG) << TAG("vm") << "Decoded `JMP`\n";
      auto [success, value] = decode_qword(instruction_data);
      if (!success) {
        LOG(FATAL) << TAG("vm") << "Failed to decode QWORD\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_jmp_c>(value));
      break;
    }
    case libskiff::bytecode::instructions::SYSCALL: {
      auto [success, value] = decode_qword(instruction_data);
      if (!success) {
        LOG(FATAL) << TAG("vm") << "Failed to decode QWORD\n";
        return false;
      }
      LOG(DEBUG) << TAG("vm") << "Decoded `SYSCALL` to " << value << "\n";
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_syscall_c>(value));
      break;
    }
    case libskiff::bytecode::instructions::DEBUG: {
      auto [success, value] = decode_qword(instruction_data);
      if (!success) {
        LOG(FATAL) << TAG("vm") << "Failed to decode QWORD\n";
        return false;
      }
      LOG(DEBUG) << TAG("vm") << "Decoded `DEBUG` with " << value << "\n";
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_debug_c>(value));
      break;
    }
    case libskiff::bytecode::instructions::FREE: {
      LOG(DEBUG) << TAG("vm") << "Decoded `FREE`\n";
      auto [success, target_register] =
          decode_ins_with_one_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_free_c>(
              *target_register));
      break;
    }
    case libskiff::bytecode::instructions::PUSH_W: {
      LOG(DEBUG) << TAG("vm") << "Decoded `PUSH_W`\n";
      auto [success, target_register] =
          decode_ins_with_one_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_push_w_c>(
              *target_register));
      break;
    }
    case libskiff::bytecode::instructions::PUSH_HW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `PUSH_HW`\n";
      auto [success, target_register] =
          decode_ins_with_one_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_push_hw_c>(
              *target_register));
      break;
    }
    case libskiff::bytecode::instructions::PUSH_DW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `PUSH_DW`\n";
      auto [success, target_register] =
          decode_ins_with_one_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_push_dw_c>(
              *target_register));
      break;
    }
    case libskiff::bytecode::instructions::PUSH_QW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `PUSH_QW`\n";
      auto [success, target_register] =
          decode_ins_with_one_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_push_qw_c>(
              *target_register));
      break;
    }
    case libskiff::bytecode::instructions::POP_W: {
      LOG(DEBUG) << TAG("vm") << "Decoded `POP_W`\n";
      auto [success, target_register] =
          decode_ins_with_one_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_pop_w_c>(
              *target_register));
      break;
    }
    case libskiff::bytecode::instructions::POP_HW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `POP_HW`\n";
      auto [success, target_register] =
          decode_ins_with_one_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_pop_hw_c>(
              *target_register));
      break;
    }
    case libskiff::bytecode::instructions::POP_DW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `POP_DW`\n";
      auto [success, target_register] =
          decode_ins_with_one_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_pop_dw_c>(
              *target_register));
      break;
    }
    case libskiff::bytecode::instructions::POP_QW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `POP_QW`\n";
      auto [success, target_register] =
          decode_ins_with_one_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_pop_qw_c>(
              *target_register));
      break;
    }
    case libskiff::bytecode::instructions::ASNE: {
      LOG(DEBUG) << TAG("vm") << "Decoded `ASNE`\n";
      auto [success, expected_reg, actual_reg] =
          decode_ins_with_two_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_asne_c>(*expected_reg,
                                                               *actual_reg));
      break;
    }
    case libskiff::bytecode::instructions::ASEQ: {
      LOG(DEBUG) << TAG("vm") << "Decoded `ASEQ`\n";
      auto [success, expected_reg, actual_reg] =
          decode_ins_with_two_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_aseq_c>(*expected_reg,
                                                               *actual_reg));
      break;
    }
    case libskiff::bytecode::instructions::NOT: {
      LOG(DEBUG) << TAG("vm") << "Decoded `NOT`\n";
      auto [success, dest, source] = decode_ins_with_two_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_not_c>(*dest, *source));
      break;
    }
    case libskiff::bytecode::instructions::ALLOC: {
      LOG(DEBUG) << TAG("vm") << "Decoded `ALLOC`\n";
      auto [success, dest, source] = decode_ins_with_two_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_alloc_c>(*dest,
                                                                *source));
      break;
    }
    case libskiff::bytecode::instructions::ADD: {
      LOG(DEBUG) << TAG("vm") << "Decoded `ADD`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_add_c>(*dest, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::SUB: {
      LOG(DEBUG) << TAG("vm") << "Decoded `SUB`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_sub_c>(*dest, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::DIV: {
      LOG(DEBUG) << TAG("vm") << "Decoded `DIV`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_div_c>(*dest, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::MUL: {
      LOG(DEBUG) << TAG("vm") << "Decoded `MUL`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_mul_c>(*dest, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::ADDF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `ADDF`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_addf_c>(*dest, *lhs,
                                                               *rhs));
      break;
    }
    case libskiff::bytecode::instructions::SUBF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `SUBF`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_subf_c>(*dest, *lhs,
                                                               *rhs));
      break;
    }
    case libskiff::bytecode::instructions::DIVF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `DIVF`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_divf_c>(*dest, *lhs,
                                                               *rhs));
      break;
    }
    case libskiff::bytecode::instructions::MULF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `MULF`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_mulf_c>(*dest, *lhs,
                                                               *rhs));
      break;
    }
    case libskiff::bytecode::instructions::LSH: {
      LOG(DEBUG) << TAG("vm") << "Decoded `LSH`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_lsh_c>(*dest, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::RSH: {
      LOG(DEBUG) << TAG("vm") << "Decoded `RSH`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_rsh_c>(*dest, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::AND: {
      LOG(DEBUG) << TAG("vm") << "Decoded `AND`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_and_c>(*dest, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::OR: {
      LOG(DEBUG) << TAG("vm") << "Decoded `OR`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_or_c>(*dest, *lhs,
                                                             *rhs));
      break;
    }
    case libskiff::bytecode::instructions::XOR: {
      LOG(DEBUG) << TAG("vm") << "Decoded `XOR`\n";
      auto [success, dest, lhs, rhs] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_xor_c>(*dest, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::SW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `SW`\n";
      auto [success, idx, offset, data] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_store_word_c>(
              *idx, *offset, *data));
      break;
    }
    case libskiff::bytecode::instructions::SHW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `SHW`\n";
      auto [success, idx, offset, data] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_store_hword_c>(
              *idx, *offset, *data));
      break;
    }
    case libskiff::bytecode::instructions::SDW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `SDW`\n";
      auto [success, idx, offset, data] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_store_dword_c>(
              *idx, *offset, *data));
      break;
    }
    case libskiff::bytecode::instructions::SQW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `SQW`\n";
      auto [success, idx, offset, data] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_store_qword_c>(
              *idx, *offset, *data));
      break;
    }
    case libskiff::bytecode::instructions::LW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `LW`\n";
      auto [success, idx, offset, data] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_load_word_c>(
              *idx, *offset, *data));
      break;
    }
    case libskiff::bytecode::instructions::LHW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `LHW`\n";
      auto [success, idx, offset, data] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_load_hword_c>(
              *idx, *offset, *data));
      break;
    }
    case libskiff::bytecode::instructions::LDW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `LDW`\n";
      auto [success, idx, offset, data] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_load_dword_c>(
              *idx, *offset, *data));
      break;
    }
    case libskiff::bytecode::instructions::LQW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `LQW`\n";
      auto [success, idx, offset, data] =
          decode_ins_with_three_reg(instruction_data);
      if (!success) {
        return false;
      }

      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_load_qword_c>(
              *idx, *offset, *data));
      break;
    }
    case libskiff::bytecode::instructions::MOV: {
      LOG(DEBUG) << TAG("vm") << "Decoded `MOV`\n";

      if (instruction_data.size() != 9) {
        LOG(FATAL) << TAG("vm")
                   << "Insufficent data to construct instruction\n";
        return false;
      }
      auto targeted_register = get_register(instruction_data[0]);
      if (!targeted_register) {
        LOG(FATAL) << TAG("vm") << "Unable to locate register by value\n";
        return false;
      }
      std::vector<uint8_t> value = {instruction_data.begin() + 1,
                                    instruction_data.end()};
      auto [success, mov_value] = decode_qword(value);
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_mov_c>(
              *targeted_register, mov_value));
      break;
    }
    case libskiff::bytecode::instructions::BGT: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BGT`\n";
      auto [success, lhs, rhs, destination] =
          decode_branch_instruction(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_bgt_c>(destination, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::BLT: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BLT`\n";
      auto [success, lhs, rhs, destination] =
          decode_branch_instruction(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_blt_c>(destination, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::BEQ: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BEQ`\n";
      auto [success, lhs, rhs, destination] =
          decode_branch_instruction(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_beq_c>(destination, *lhs,
                                                              *rhs));
      break;
    }
    case libskiff::bytecode::instructions::BGTF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BGTF`\n";
      auto [success, lhs, rhs, destination] =
          decode_branch_instruction(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_bgtf_c>(destination,
                                                               *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::BLTF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BLTF`\n";
      auto [success, lhs, rhs, destination] =
          decode_branch_instruction(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_bltf_c>(destination,
                                                               *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::BEQF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BEQF`\n";
      auto [success, lhs, rhs, destination] =
          decode_branch_instruction(instruction_data);
      if (!success) {
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<skiff::machine::instruction_beqf_c>(destination,
                                                               *lhs, *rhs));
      break;
    }
    }
  }
  _runtime_data.instructions_loaded = _instructions.size();
  return true;
}

types::vm_register *vm_c::get_register(uint8_t id)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  LOG(DEBUG) << TAG("vm") << "Request for register : " << std::hex
             << static_cast<int>(id) << std::dec << "\n";
  switch (id) {
  case 0x00:
    return &_x0;
    break;
  case 0x01:
    return &_x1;
    break;
  case 0x02:
    return &_ip;
    break;
  case 0x03:
    return &_sp;
    break;
  case 0x10:
    return &_integer_registers[0];
    break;
  case 0x11:
    return &_integer_registers[1];
    break;
  case 0x12:
    return &_integer_registers[2];
    break;
  case 0x13:
    return &_integer_registers[3];
    break;
  case 0x14:
    return &_integer_registers[4];
    break;
  case 0x15:
    return &_integer_registers[5];
    break;
  case 0x16:
    return &_integer_registers[6];
    break;
  case 0x17:
    return &_integer_registers[7];
    break;
  case 0x18:
    return &_integer_registers[8];
    break;
  case 0x19:
    return &_integer_registers[9];
    break;
  case 0x20:
    return &_floating_point_registers[0];
    break;
  case 0x21:
    return &_floating_point_registers[1];
    break;
  case 0x22:
    return &_floating_point_registers[2];
    break;
  case 0x23:
    return &_floating_point_registers[3];
    break;
  case 0x24:
    return &_floating_point_registers[4];
    break;
  case 0x25:
    return &_floating_point_registers[5];
    break;
  case 0x26:
    return &_floating_point_registers[6];
    break;
  case 0x27:
    return &_floating_point_registers[7];
    break;
  case 0x28:
    return &_floating_point_registers[8];
    break;
  case 0x29:
    return &_floating_point_registers[9];
    break;
  case 0xFF:
    return &_op_register;
    break;
  default:
    return nullptr;
  }
}

} // namespace machine
} // namespace skiff
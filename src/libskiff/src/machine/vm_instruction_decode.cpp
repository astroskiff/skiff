#include "libskiff/bytecode/instructions.hpp"
#include "libskiff/defines.hpp"
#include "libskiff/logging/aixlog.hpp"
#include "libskiff/machine/vm.hpp"
#include "libskiff/types.hpp"
#include "libskiff/version.hpp"

namespace libskiff {
namespace machine {
/*
    Load the binary into a series of objects that can visit the VM for
   execution. Pre-decoding the instructions this way saves us the time of
   splitting them up at execution time and lets us pre-check the binary for any
   illegal instructions before execution.

    These structures store references to registers as well meaning we won't need
    to determine where to place data at run time.
*/
bool vm_c::load(std::unique_ptr<libskiff::binary::executable_c> executable)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  // Check if its experimental
  if (executable->is_experimental()) {
    issue_forced_warning("Code marked experimental");
  }

  // Check compatibilty
  auto version = executable->get_compatiblity_semver();

  LOG(DEBUG) << TAG("WHAT") << "semver.major:"
             << (int)libskiff::version::semantic_version.minor
             << "thisversion.major:  " << (int)version.minor << "\n";

  if (libskiff::version::semantic_version.major < version.major) {
    issue_forced_error("Incompatibility detected : "
                       "Bytecode version.major newer than VM version.major. ");
    return false;
  }
  if (libskiff::version::semantic_version.minor < version.minor) {

    LOG(DEBUG) << TAG("WHAT") << "semver.minor:"
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

  // Create instructions - return false if illegal instruction found
  auto instructions = executable->get_instructions();
  auto num_instructions = instructions.size() / 8;
  _instructions.reserve(num_instructions);

  uint64_t instruction_idx{0x00};
  for (auto i = 0; i < num_instructions; i++) {

    // Pull 8 bytes that will make up a single instruction
    uint8_t shift = 56;
    uint64_t instruction{0x00};
    auto next_end = instruction_idx + 8;
    for (; instruction_idx < next_end; instruction_idx++) {
      instruction |=
          (static_cast<uint64_t>(instructions[instruction_idx]) << shift);
      shift -= 8;
    }

    uint32_t instruction_top = instruction >> 32;
    uint32_t instruction_bot = instruction;

    // Figure out what instruction it is, and decode it into an ec class
    switch (instruction_top & 0xFF) {
    case libskiff::bytecode::instructions::NOP: {
      LOG(DEBUG) << TAG("vm") << "Decoded `NOP`\n";
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_nop_c>());
      break;
    }
    case libskiff::bytecode::instructions::EXIT: {
      LOG(DEBUG) << TAG("vm") << "Decoded `EXIT`\n";
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_exit_c>());
      break;
    }
    case libskiff::bytecode::instructions::RET: {
      LOG(DEBUG) << TAG("vm") << "Decoded `RET`\n";
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_ret_c>());
      break;
    }
    case libskiff::bytecode::instructions::JMP: {
      LOG(DEBUG) << TAG("vm") << "Decoded `JMP`\n";
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_jmp_c>(
              instruction_bot));
      break;
    }
    case libskiff::bytecode::instructions::CALL: {
      LOG(DEBUG) << TAG("vm") << "Decoded `CALL`\n";
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_call_c>(
              instruction_bot));
      break;
    }
    case libskiff::bytecode::instructions::BLT: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BLT`\n";
      auto lhs = get_register(instruction_top >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_top >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_blt_c>(
              instruction_bot, *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::BGT: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BGT`\n";
      auto lhs = get_register(instruction_top >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_top >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_bgt_c>(
              instruction_bot, *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::BEQ: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BEQ`\n";
      auto lhs = get_register(instruction_top >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_top >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_beq_c>(
              instruction_bot, *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::MOV: {
      LOG(DEBUG) << TAG("vm") << "Decoded `MOV`\n";
      auto dest_reg = get_register(instruction_top >> 8);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_mov_c>(
              *dest_reg, instruction_bot));
      break;
    }
    case libskiff::bytecode::instructions::ADD: {
      LOG(DEBUG) << TAG("vm") << "Decoded `ADD`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_add_c>(*dest_reg,
                                                                 *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::SUB: {
      LOG(DEBUG) << TAG("vm") << "Decoded `SUB`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_sub_c>(*dest_reg,
                                                                 *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::DIV: {
      LOG(DEBUG) << TAG("vm") << "Decoded `DIV`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_div_c>(*dest_reg,
                                                                 *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::MUL: {
      LOG(DEBUG) << TAG("vm") << "Decoded `MUL`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_mul_c>(*dest_reg,
                                                                 *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::ADDF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `ADDF`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_addf_c>(*dest_reg,
                                                                  *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::SUBF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `SUBF`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_subf_c>(*dest_reg,
                                                                  *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::DIVF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `DIVF`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_divf_c>(*dest_reg,
                                                                  *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::MULF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `MULF`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_mulf_c>(*dest_reg,
                                                                  *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::RSH: {
      LOG(DEBUG) << TAG("vm") << "Decoded `RSH`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_rsh_c>(*dest_reg,
                                                                 *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::LSH: {
      LOG(DEBUG) << TAG("vm") << "Decoded `LSH`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_lsh_c>(*dest_reg,
                                                                 *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::AND: {
      LOG(DEBUG) << TAG("vm") << "Decoded `AND`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_and_c>(*dest_reg,
                                                                 *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::OR: {
      LOG(DEBUG) << TAG("vm") << "Decoded `OR`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_or_c>(*dest_reg, *lhs,
                                                                *rhs));
      break;
    }
    case libskiff::bytecode::instructions::XOR: {
      LOG(DEBUG) << TAG("vm") << "Decoded `XOR`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto lhs = get_register(instruction_bot >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_bot >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_xor_c>(*dest_reg,
                                                                 *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::NOT: {
      LOG(DEBUG) << TAG("vm") << "Decoded `NOT`\n";
      auto dest_reg = get_register(instruction_bot >> 24);
      if (!dest_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode DEST register`\n";
        return false;
      }
      auto source = get_register(instruction_bot >> 16);
      if (!source) {
        LOG(FATAL) << TAG("vm") << "Unable to decode SOURCE register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_not_c>(*dest_reg,
                                                                 *source));
      break;
    }
    case libskiff::bytecode::instructions::BLTF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BLTF`\n";
      auto lhs = get_register(instruction_top >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_top >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_bltf_c>(
              instruction_bot, *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::BGTF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BGTF`\n";
      auto lhs = get_register(instruction_top >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_top >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_bgtf_c>(
              instruction_bot, *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::BEQF: {
      LOG(DEBUG) << TAG("vm") << "Decoded `BEQF`\n";
      auto lhs = get_register(instruction_top >> 16);
      if (!lhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode LHS register`\n";
        return false;
      }
      auto rhs = get_register(instruction_top >> 8);
      if (!rhs) {
        LOG(FATAL) << TAG("vm") << "Unable to decode RHS register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_beqf_c>(
              instruction_bot, *lhs, *rhs));
      break;
    }
    case libskiff::bytecode::instructions::ASNE: {
      LOG(DEBUG) << TAG("vm") << "Decoded `ASNE`\n";
      auto expected_reg = get_register(instruction_bot >> 24);
      if (!expected_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode EXPECTED register`\n";
        return false;
      }
      auto actual_reg = get_register(instruction_bot >> 16);
      if (!actual_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode ACTUAL register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_asne_c>(*expected_reg,
                                                                  *actual_reg));
      break;
    }
    case libskiff::bytecode::instructions::ASEQ: {
      LOG(DEBUG) << TAG("vm") << "Decoded `ASEQ`\n";
      auto expected_reg = get_register(instruction_bot >> 24);
      if (!expected_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode EXPECTED register`\n";
        return false;
      }
      auto actual_reg = get_register(instruction_bot >> 16);
      if (!actual_reg) {
        LOG(FATAL) << TAG("vm") << "Unable to decode ACTUAL register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_aseq_c>(*expected_reg,
                                                                  *actual_reg));
      break;
    }
    case libskiff::bytecode::instructions::PUSH_W: {
      LOG(DEBUG) << TAG("vm") << "Decoded `PUSH_W`\n";
      auto source = get_register(instruction_bot >> 24);
      if (!source) {
        LOG(FATAL) << TAG("vm") << "Unable to decode SOURCE register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_push_w_c>(*source));
      break;
    }
    case libskiff::bytecode::instructions::PUSH_DW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `PUSH_DW`\n";
      auto source = get_register(instruction_bot >> 24);
      if (!source) {
        LOG(FATAL) << TAG("vm") << "Unable to decode SOURCE register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_push_dw_c>(*source));
      break;
    }
    case libskiff::bytecode::instructions::PUSH_QW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `PUSH_QW`\n";
      auto source = get_register(instruction_bot >> 24);
      if (!source) {
        LOG(FATAL) << TAG("vm") << "Unable to decode SOURCE register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_push_qw_c>(*source));
      break;
    }
    case libskiff::bytecode::instructions::POP_W: {
      LOG(DEBUG) << TAG("vm") << "Decoded `POP_W`\n";
      auto dest = get_register(instruction_bot >> 24);
      if (!dest) {
        LOG(FATAL) << TAG("vm") << "Unable to decode SOURCE register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_pop_w_c>(*dest));
      break;
    }
    case libskiff::bytecode::instructions::POP_DW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `POP_DW`\n";
      auto dest = get_register(instruction_bot >> 24);
      if (!dest) {
        LOG(FATAL) << TAG("vm") << "Unable to decode SOURCE register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_pop_dw_c>(*dest));
      break;
    }
    case libskiff::bytecode::instructions::POP_QW: {
      LOG(DEBUG) << TAG("vm") << "Decoded `POP_QW`\n";
      auto dest = get_register(instruction_bot >> 24);
      if (!dest) {
        LOG(FATAL) << TAG("vm") << "Unable to decode SOURCE register`\n";
        return false;
      }
      _instructions.emplace_back(
          std::make_unique<libskiff::machine::instruction_pop_qw_c>(*dest));
      break;
    }
    }
  }

  // Create memory module
  return true;
}

types::vm_register *vm_c::get_register(uint8_t id)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  LOG(DEBUG) << TAG("vm") << "Request for register : " << static_cast<int>(id)
             << "\n";
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
    return &_fp;
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
} // namespace libskiff
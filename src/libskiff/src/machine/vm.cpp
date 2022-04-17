#include "libskiff/machine/vm.hpp"
#include "libskiff/bytecode/floating_point.hpp"
#include "libskiff/bytecode/instructions.hpp"
#include "libskiff/defines.hpp"
#include "libskiff/logging/aixlog.hpp"
#include "libskiff/machine/system/callable.hpp"
#include "libskiff/machine/system/print.hpp"
#include "libskiff/types.hpp"
#include "libskiff/version.hpp"
#include <iostream>

namespace libskiff {
namespace machine {

namespace {
static inline void force_debug(const std::string &msg)
{
  std::cout << TERM_COLOR_CYAN << "[DEBUG] : " << TERM_COLOR_END << msg
            << std::endl;
}
} // namespace

vm_c::vm_c()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  _stack.set_sp(_sp);

  //  Setup system calls
  //  - Order here matters as their index will determine what
  //    system call number they are so we don't need to register them and
  //    go through a slow map

  _system_callables.emplace_back(new system::print_c()); // Syscall 0
}

vm_c::~vm_c() {}

// Force warning to screen and logger
void vm_c::issue_forced_warning(const std::string &warn)
{
  std::cout << TERM_COLOR_YELLOW << "[WARNING] : " << TERM_COLOR_END << warn
            << std::endl;
  LOG(WARNING) << TAG("vm") << warn << "\n";
}
// Force error to screen and logger
void vm_c::issue_forced_error(const std::string &err)
{
  std::cout << TERM_COLOR_RED << "[ERROR] : " << TERM_COLOR_END << err
            << std::endl;
  LOG(WARNING) << TAG("vm") << err << "\n";
}

void vm_c::set_runtime_callback(libskiff::types::runtime_error_cb cb)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  _runtime_error_cb = {cb};
}

std::pair<vm_c::execution_result_e, int> vm_c::execute()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  while (_is_alive) {

    // Ensure that the instruction pointer isn't wack
    if (_ip > _instructions.size() || _ip < 0) {
      std::string msg =
          "Instruction pointer out of range : " + std::to_string(_ip);
      kill_with_error(
          libskiff::types::runtime_error_e::INSTRUCTION_PTR_OUT_OF_RANGE, msg);
      continue;
    }

    // Update registers
    _x0 = 0; // Constant 0
    _x1 = 1; // Constant 1

    // Execute the instruction
    _instructions[_ip]->visit(*this);
  }

  // Return back with the return status and exit code
  return {_return_value, _integer_registers[0]};
}

void vm_c::kill_with_error(const types::runtime_error_e err,
                           const std::string &err_str)
{
  _is_alive = false;
  _integer_registers[0] = 1;
  _return_value = execution_result_e::ERROR;
  if (_runtime_error_cb != std::nullopt) {
    (*_runtime_error_cb)(err);
  }
  LOG(DEBUG) << TAG("vm") << "Kill issued @ip = " << _ip << ": " << err_str
             << "\n";
}

void vm_c::accept(instruction_nop_c &ins)
{
  switch (_debug_level) {
  case libskiff::types::exec_debug_level_e::NONE:
    break;
  default:
    force_debug("NOP Instruction @ IP = " + std::to_string(_ip));
    break;
  }
  _ip++;
}

void vm_c::accept(instruction_exit_c &ins)
{
  switch (_debug_level) {
  case libskiff::types::exec_debug_level_e::NONE:
    break;
  default:
    force_debug("EXIT Instruction @ IP = " + std::to_string(_ip));
    break;
  }
  _is_alive = false;
  _ip++;
}

void vm_c::accept(instruction_blt_c &ins)
{
  if (ins.lhs_reg < ins.rhs_reg) {
    _ip = ins.destination;
  }
  else {
    _ip++;
  }
}

void vm_c::accept(instruction_bgt_c &ins)
{
  if (ins.lhs_reg > ins.rhs_reg) {
    _ip = ins.destination;
  }
  else {
    _ip++;
  }
}

void vm_c::accept(instruction_beq_c &ins)
{
  if (ins.lhs_reg == ins.rhs_reg) {
    _ip = ins.destination;
  }
  else {
    _ip++;
  }
}

void vm_c::accept(instruction_jmp_c &ins) { _ip = ins.destination; }

void vm_c::accept(instruction_call_c &ins)
{
  _call_stack.push(_ip + 1);
  _ip = ins.destination;
}

void vm_c::accept(instruction_ret_c &ins)
{
  if (_call_stack.empty()) {
    // Kill the run, set 1 as exit result, inform the owner
    kill_with_error(
        libskiff::types::runtime_error_e::RETURN_WITH_EMPTY_CALLSTACK,
        "`ret` instruction hit with empty callstack");
    return;
  }

  _ip = _call_stack.top();
  _call_stack.pop();
}

void vm_c::accept(instruction_mov_c &ins)
{
  // issue_forced_warning("MOV : Move is only moving 32 bits and is not directly
  // 'moving' constants into place. ");
  ins.dest_reg = ins.value;
  _ip++;
}

void vm_c::accept(instruction_add_c &ins)
{
  ins.dest_reg = ins.lhs_reg + ins.rhs_reg;
  _ip++;
}

void vm_c::accept(instruction_sub_c &ins)
{
  ins.dest_reg = ins.lhs_reg - ins.rhs_reg;
  _ip++;
}

void vm_c::accept(instruction_div_c &ins)
{
  if (ins.rhs_reg == 0) {
    kill_with_error(libskiff::types::runtime_error_e::DIVIDE_BY_ZERO,
                    "`div` instruction asked to divide by 0");
    return;
  }
  ins.dest_reg = ins.lhs_reg / ins.rhs_reg;
  _ip++;
}

void vm_c::accept(instruction_mul_c &ins)
{
  ins.dest_reg = ins.lhs_reg * ins.rhs_reg;
  _ip++;
}

void vm_c::accept(instruction_addf_c &ins)
{
  ins.dest_reg = libskiff::bytecode::floating_point::to_uint64_t(
      libskiff::bytecode::floating_point::from_uint64_t(ins.lhs_reg) +
      libskiff::bytecode::floating_point::from_uint64_t(ins.rhs_reg));
  _ip++;
}

void vm_c::accept(instruction_subf_c &ins)
{
  ins.dest_reg = libskiff::bytecode::floating_point::to_uint64_t(
      libskiff::bytecode::floating_point::from_uint64_t(ins.lhs_reg) -
      libskiff::bytecode::floating_point::from_uint64_t(ins.rhs_reg));
  _ip++;
}

void vm_c::accept(instruction_divf_c &ins)
{
  if (libskiff::bytecode::floating_point::are_equal(ins.rhs_reg, 0.0)) {
    kill_with_error(libskiff::types::runtime_error_e::DIVIDE_BY_ZERO,
                    "`divf` instruction asked to divide by 0");
    return;
  }
  ins.dest_reg = libskiff::bytecode::floating_point::to_uint64_t(
      libskiff::bytecode::floating_point::from_uint64_t(ins.lhs_reg) /
      libskiff::bytecode::floating_point::from_uint64_t(ins.rhs_reg));
  _ip++;
}

void vm_c::accept(instruction_mulf_c &ins)
{
  ins.dest_reg = libskiff::bytecode::floating_point::to_uint64_t(
      libskiff::bytecode::floating_point::from_uint64_t(ins.lhs_reg) *
      libskiff::bytecode::floating_point::from_uint64_t(ins.rhs_reg));
  _ip++;
}

void vm_c::accept(instruction_lsh_c &ins)
{
  ins.dest_reg = ins.lhs_reg << ins.rhs_reg;
  _ip++;
}

void vm_c::accept(instruction_rsh_c &ins)
{
  ins.dest_reg = ins.lhs_reg >> ins.rhs_reg;
  _ip++;
}

void vm_c::accept(instruction_and_c &ins)
{
  ins.dest_reg = ins.lhs_reg & ins.rhs_reg;
  _ip++;
}
void vm_c::accept(instruction_or_c &ins)
{
  ins.dest_reg = ins.lhs_reg | ins.rhs_reg;
  _ip++;
}

void vm_c::accept(instruction_xor_c &ins)
{
  ins.dest_reg = ins.lhs_reg ^ ins.rhs_reg;
  _ip++;
}

void vm_c::accept(instruction_not_c &ins)
{
  ins.dest_reg = !ins.source_reg;
  _ip++;
}

void vm_c::accept(instruction_bltf_c &ins)
{
  if (libskiff::bytecode::floating_point::from_uint64_t(ins.lhs_reg) <
      libskiff::bytecode::floating_point::from_uint64_t(ins.rhs_reg)) {
    _ip = ins.destination;
  }
  else {
    _ip++;
  }
}

void vm_c::accept(instruction_bgtf_c &ins)
{
  if (libskiff::bytecode::floating_point::from_uint64_t(ins.lhs_reg) >
      libskiff::bytecode::floating_point::from_uint64_t(ins.rhs_reg)) {
    _ip = ins.destination;
  }
  else {
    _ip++;
  }
}

void vm_c::accept(instruction_beqf_c &ins)
{
  if (libskiff::bytecode::floating_point::from_uint64_t(ins.lhs_reg) ==
      libskiff::bytecode::floating_point::from_uint64_t(ins.rhs_reg)) {
    _ip = ins.destination;
  }
  else {
    _ip++;
  }
}

void vm_c::accept(instruction_asne_c &ins)
{
  if (ins.expected_reg == ins.actual_reg) {
    LOG(DEBUG) << TAG("vm") << TERM_COLOR_RED
               << "Assertion `ASNE` failed! Expected [" << ins.expected_reg
               << "] Actual [" << ins.actual_reg << "]" << TERM_COLOR_END
               << "\n";
    _integer_registers[0] = 1;
    _is_alive = false;
  }
  _ip++;
}

void vm_c::accept(instruction_aseq_c &ins)
{
  if (ins.expected_reg != ins.actual_reg) {
    LOG(DEBUG) << TAG("vm") << TERM_COLOR_RED
               << "Assertion `ASEQ` failed! Expected [" << ins.expected_reg
               << "] Actual [" << ins.actual_reg << "]" << TERM_COLOR_END
               << "\n";
    _integer_registers[0] = 1;
    _is_alive = false;
  }
  _ip++;
}

void vm_c::accept(instruction_push_w_c &ins)
{
  if (!_stack.push_word(ins.source)) {
    kill_with_error(libskiff::types::runtime_error_e::STACK_PUSH_ERROR,
                    "Unable to push data to stack. Out of memory?");
  }
  _ip++;
}

void vm_c::accept(instruction_push_dw_c &ins)
{
  if (!_stack.push_dword(ins.source)) {
    kill_with_error(libskiff::types::runtime_error_e::STACK_PUSH_ERROR,
                    "Unable to push data to stack. Out of memory?");
  }
  _ip++;
}

void vm_c::accept(instruction_push_qw_c &ins)
{
  if (!_stack.push_qword(ins.source)) {
    kill_with_error(libskiff::types::runtime_error_e::STACK_PUSH_ERROR,
                    "Unable to push data to stack. Out of memory?");
  }
  _ip++;
}

void vm_c::accept(instruction_pop_w_c &ins)
{
  auto [okay, value] = _stack.pop_word();
  if (!okay) {
    kill_with_error(libskiff::types::runtime_error_e::STACK_POP_ERROR,
                    "Unable to pop data from stack. Stack empty?");
  }
  ins.dest = value;
  _ip++;
}

void vm_c::accept(instruction_pop_dw_c &ins)
{
  auto [okay, value] = _stack.pop_dword();
  if (!okay) {
    kill_with_error(libskiff::types::runtime_error_e::STACK_POP_ERROR,
                    "Unable to pop data from stack. Stack empty?");
  }
  ins.dest = value;
  _ip++;
}

void vm_c::accept(instruction_pop_qw_c &ins)
{
  auto [okay, value] = _stack.pop_qword();
  if (!okay) {
    kill_with_error(libskiff::types::runtime_error_e::STACK_POP_ERROR,
                    "Unable to pop data from stack. Stack empty?");
  }
  ins.dest = value;
  _ip++;
}

void vm_c::accept(instruction_alloc_c &ins)
{
  auto [okay, value] = _memman.alloc(ins.size);
  if (!okay) {
    _op_register = 0;
  }
  else {
    ins.dest = value;
    _op_register = 1;
  }
  _ip++;
}

void vm_c::accept(instruction_free_c &ins)
{
  if (!_memman.free(ins.idx)) {
    _op_register = 0;
  }
  else {
    _op_register = 1;
  }
  _ip++;
}

void vm_c::accept(instruction_store_word_c &ins)
{
  auto slot = _memman.get_slot(ins.idx);
  _ip++;

  if (!slot) {
    _op_register = 0;
    return;
  }

  if (!slot->put_word(ins.offset, ins.data)) {
    _op_register = 0;
    return;
  }

  _op_register = 1;
}

void vm_c::accept(instruction_store_dword_c &ins)
{
  auto slot = _memman.get_slot(ins.idx);
  _ip++;

  if (!slot) {
    _op_register = 0;
    return;
  }

  if (!slot->put_dword(ins.offset, ins.data)) {
    _op_register = 0;
    return;
  }

  _op_register = 1;
}

void vm_c::accept(instruction_store_qword_c &ins)
{
  auto slot = _memman.get_slot(ins.idx);
  _ip++;

  if (!slot) {
    _op_register = 0;
    return;
  }

  if (!slot->put_qword(ins.offset, ins.data)) {
    _op_register = 0;
    return;
  }

  _op_register = 1;
}

void vm_c::accept(instruction_load_word_c &ins)
{
  auto slot = _memman.get_slot(ins.idx);
  _ip++;

  if (!slot) {
    _op_register = 0;
    return;
  }

  auto [okay, value] = slot->get_word(ins.offset);
  if (!okay) {
    _op_register = 0;
    return;
  }
  _op_register = 1;
  ins.dest = value;
}

void vm_c::accept(instruction_load_dword_c &ins)
{
  auto slot = _memman.get_slot(ins.idx);
  _ip++;

  if (!slot) {
    _op_register = 0;
    return;
  }

  auto [okay, value] = slot->get_dword(ins.offset);
  if (!okay) {
    _op_register = 0;
    return;
  }
  _op_register = 1;
  ins.dest = value;
}

void vm_c::accept(instruction_load_qword_c &ins)
{
  auto slot = _memman.get_slot(ins.idx);
  _ip++;

  if (!slot) {
    _op_register = 0;
    return;
  }

  auto [okay, value] = slot->get_qword(ins.offset);
  if (!okay) {
    _op_register = 0;
    return;
  }
  _op_register = 1;
  ins.dest = value;
}

void vm_c::accept(instruction_syscall_c &ins)
{
  _ip++;

  // Ensure that the requested item is within range of callables
  if (ins.address >= _system_callables.size()) {
    _op_register = 0;
    return;
  }

  // Construct the view into the vm
  types::view_t view = {.integer_registers = _integer_registers,
                        .float_registers = _floating_point_registers,
                        .memory_manager = _memman,
                        .op_register = _op_register};

  // Call the item
  _system_callables[ins.address]->execute(view);
}

void vm_c::accept(instruction_debug_c &ins)
{
  _ip++;

  std::cout << TERM_COLOR_BRIGHT_YELLOW << "DEBUG INS:" << ins.id << TERM_COLOR_END << std::endl;

  switch (_debug_level) {
  case libskiff::types::exec_debug_level_e::NONE:
    break;
  case libskiff::types::exec_debug_level_e::MINIMAL:
    std::cout << "ip | " << _ip << std::endl;
    break;
  case libskiff::types::exec_debug_level_e::EXTREME:
    std::cout << "Integer Registers" << std::endl;
    for(auto i = 0; i < config::num_integer_registers; i++) {
      std::cout << "i" << i << " | " << _integer_registers[i] << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Float Registers" << std::endl;
    for(auto i = 0; i < config::num_floating_point_registers; i++) {
      std::cout << "f" << i << " | " << _floating_point_registers[i] << std::endl;
    }
    std::cout << std::endl;
  case libskiff::types::exec_debug_level_e::MODERATE:
    std::cout << "System Registers" << std::endl;
    std::cout << "x0 | " << _x0 << std::endl 
              << "x1 | " << _x1 << std::endl 
              << "sp | " << _sp << std::endl 
              << "ip | " << _ip << std::endl 
              << "op | " << _op_register << std::endl;
    std::cout << std::endl;
    break;
  }
}

} // namespace machine
} // namespace libskiff
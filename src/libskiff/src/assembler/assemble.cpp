
#include "libskiff/assembler/assemble.hpp"
#include "libskiff/bytecode/instructions.hpp"
#include "libskiff/generators/binary_generator.hpp"
#include "libskiff/generators/instruction_generator.hpp"
#include "libskiff/logging/aixlog.hpp"
#include "libskiff/types.hpp"

#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <unordered_map>

namespace libskiff {
namespace assembler {
namespace {

inline std::unordered_map<std::string, uint8_t> get_string_to_instruction_map()
{
  return {{"nop", libskiff::bytecode::instructions::NOP},
          {"exit", libskiff::bytecode::instructions::EXIT},
          {"blt", libskiff::bytecode::instructions::BLT},
          {"bgt", libskiff::bytecode::instructions::BGT},
          {"beq", libskiff::bytecode::instructions::BEQ},
          {"jmp", libskiff::bytecode::instructions::JMP},
          {"call", libskiff::bytecode::instructions::CALL},
          {"ret", libskiff::bytecode::instructions::RET},
          {"mov", libskiff::bytecode::instructions::MOV},
          {"add", libskiff::bytecode::instructions::ADD},
          {"sub", libskiff::bytecode::instructions::SUB},
          {"div", libskiff::bytecode::instructions::DIV},
          {"mul", libskiff::bytecode::instructions::MUL},
          {"addf", libskiff::bytecode::instructions::ADDF},
          {"subf", libskiff::bytecode::instructions::SUBF},
          {"divf", libskiff::bytecode::instructions::DIVF},
          {"mulf", libskiff::bytecode::instructions::MULF},
          {"lsh", libskiff::bytecode::instructions::LSH},
          {"rsh", libskiff::bytecode::instructions::RSH},
          {"and", libskiff::bytecode::instructions::AND},
          {"or", libskiff::bytecode::instructions::OR},
          {"xor", libskiff::bytecode::instructions::XOR},
          {"not", libskiff::bytecode::instructions::NOT},
          {"bltf", libskiff::bytecode::instructions::BLTF},
          {"bgtf", libskiff::bytecode::instructions::BGTF},
          {"beqf", libskiff::bytecode::instructions::BEQF},
          {"aseq", libskiff::bytecode::instructions::ASEQ},
          {"asne", libskiff::bytecode::instructions::ASNE},
          {"push_w", libskiff::bytecode::instructions::PUSH_W},
          {"push_dw", libskiff::bytecode::instructions::PUSH_DW},
          {"push_qw", libskiff::bytecode::instructions::PUSH_QW},
          {"pop_w", libskiff::bytecode::instructions::POP_W},
          {"pop_dw", libskiff::bytecode::instructions::POP_DW},
          {"pop_qw", libskiff::bytecode::instructions::POP_QW},
          {"alloc", libskiff::bytecode::instructions::ALLOC},
          {"free", libskiff::bytecode::instructions::FREE},
          {"sw", libskiff::bytecode::instructions::SW},
          {"sdw", libskiff::bytecode::instructions::SDW},
          {"sqw", libskiff::bytecode::instructions::SQW},
          {"lw", libskiff::bytecode::instructions::LW},
          {"ldw", libskiff::bytecode::instructions::LDW},
          {"lqw", libskiff::bytecode::instructions::LQW},
          {"syscall", libskiff::bytecode::instructions::SYSCALL},
          {"debug", libskiff::bytecode::instructions::DEBUG},
          {"eirq", libskiff::bytecode::instructions::EIRQ},
          {"dirq", libskiff::bytecode::instructions::DIRQ}};
}

template <class T> std::optional<T> get_number(const std::string value)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  T value_out = 0;
  std::stringstream ss(value);
  ss >> value_out;
  if (ss.fail()) {
    return std::nullopt;
  }
  return value_out;
}

std::tuple<bool, std::string> get_string_literal(std::string value)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  if (value.size() < 3) {
    return {false, {}};
  }
  if (value.front() != '"' || value.back() != '"') {
    return {false, {}};
  }
  return {true, value.substr(1, value.size() - 2)};
}

inline std::vector<std::string> chunk_line(std::string line)
{
  std::vector<std::string> chunks;
  bool in_sub_str = false;
  std::string token = "";
  if (!line.empty() && line[line.size() - 1] == '\r') {
    line.erase(line.size() - 1);
  }
  for (unsigned i = 0; i < line.length(); i++) {
    if (line[i] == '"') {
      in_sub_str ? in_sub_str = false : in_sub_str = true;
    }
    if ((line[i] == ' ' || line[i] == '\t' || line[i] == '\n') && !in_sub_str) {
      if (token.length() > 0) {
        chunks.push_back(token);
        token.clear();
      }
    }
    else {
      token = token + line[i];
    }
  }
  if (token.length() > 0) {
    chunks.push_back(token);
  }
  return chunks;
}

struct constant_value_t {
  libskiff::types::constant_type_e type;
  uint64_t address;
  uint64_t data_len;
};

struct line_data_t {
  uint64_t line_number;
  std::vector<std::string> pieces;
};

struct instruction_data_t {
  uint8_t instruction_length{0}; // Instruction length in bytes
  uint8_t instruction_id{0};     // Instruction id
  line_data_t line_data;         // Line data
};

struct assembler_data_t {
  assembled_t result;
  std::set<uint64_t> interrupts;
  libskiff::generator::binary_generator bin_generator;
  libskiff::generator::instruction_generator_c ins_generator;
  std::unordered_map<std::string, uint64_t> label_to_instruction_address;
  std::vector<instruction_data_t> instructions_to_parse;
  std::vector<std::tuple<uint64_t, std::string>> raw_directives;
  std::unordered_map<std::string, constant_value_t> constant_name_to_meta;
  uint64_t loaded_const_address{0};
  bool init_found{false};
  bool code_found{false};
  bool debug_found{false};
};

} // namespace

void add_issue(const std::string location, const std::string phase,
               const std::string str, assembler_data_t &adt, bool is_error)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  std::string prompt = "Warning: ";
  if (is_error) {
    prompt = "Error: ";
  }

  std::string e;
  if (!location.empty()) {
    e += "Location: " + location + ", ";
  }
  e += "Phase: " + phase;
  e += ", " + prompt + str;

  if (is_error) {
    if (adt.result.errors == std::nullopt) {
      std::vector<std::string> errors;
      errors.push_back(e);
      adt.result.errors = {errors};
    }
    else {
      adt.result.errors.value().push_back(e);
    }

    LOG(FATAL) << TAG("assembler") << e << "\n";
  }
  else {
    if (adt.result.warnings == std::nullopt) {
      std::vector<std::string> warnings;
      warnings.push_back(e);
      adt.result.warnings = {warnings};
      return;
    }
    adt.result.warnings.value().push_back(e);

    LOG(WARNING) << TAG("assembler") << e << "\n";
  }
}

std::optional<uint64_t> get_label_address(const std::string &label,
                                          assembler_data_t &adt)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  if (adt.label_to_instruction_address.find(label) ==
      adt.label_to_instruction_address.end()) {
    return std::nullopt;
  }
  return adt.label_to_instruction_address[label];
}

bool directive_init(const uint64_t line_number, const std::string &line,
                    assembler_data_t &adt)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  auto chunks = chunk_line(line);
  std::string location_info = "line" + std::to_string(line_number);

  if (adt.init_found) {
    add_issue(location_info, "phase 3", "Duplicate .init directive", adt, true);
    return false;
  }
  adt.init_found = true;

  if (chunks.size() != 2) {
    add_issue(location_info, "phase 3", "Malformed .init", adt, true);
    return false;
  }

  auto init_address = get_label_address(chunks[1], adt);
  if (init_address == std::nullopt) {
    add_issue(location_info, "phase 3",
              "Unknown address for label given to .init", adt, true);
    return false;
  }

  adt.bin_generator.set_entry(*init_address);
  return true;
}

bool directive_debug(const uint64_t line_number, const std::string &line,
                     assembler_data_t &adt)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  std::string location_info = "line" + std::to_string(line_number);
  auto chunks = chunk_line(line);

  if (adt.debug_found) {
    add_issue(location_info, "phase 3", "Duplicate .debug directive", adt,
              true);
    return false;
  }

  if (chunks.size() != 2) {
    add_issue(location_info, "phase 3", "Malformed .debug directive", adt,
              true);
    return false;
  }

  auto value = get_number<int>(chunks[1]);
  if (value == std::nullopt) {
    add_issue(location_info, "phase 3", "Malformed value for .debug directive",
              adt, true);
    return false;
  }

  auto converted = static_cast<libskiff::types::exec_debug_level_e>(*value);
  switch (converted) {
  case libskiff::types::exec_debug_level_e::NONE:
    [[fallthrough]];
  case libskiff::types::exec_debug_level_e::MINIMAL:
    [[fallthrough]];
  case libskiff::types::exec_debug_level_e::MODERATE:
    [[fallthrough]];
  case libskiff::types::exec_debug_level_e::EXTREME:
    break;
  default:
    add_issue(location_info, "phase 3",
              "Invalid level given to .debug directive", adt, true);
    return false;
  }

  LOG(DEBUG) << TAG("assembler") << "Set .debug level to " << *value << "\n";

  adt.bin_generator.set_debug(converted);
  adt.debug_found = true;
  return true;
}

bool add_constant(std::string name, libskiff::types::constant_type_e type,
                  std::vector<uint8_t> data, assembler_data_t &adt)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  if (adt.constant_name_to_meta.find(name) != adt.constant_name_to_meta.end()) {
    add_issue({}, "phase 3", "Duplicate constant name '" + name + "'", adt,
              true);
    return false;
  }

  adt.bin_generator.add_constant(type, data);

  auto current_address = adt.loaded_const_address;
  adt.loaded_const_address += data.size();

  auto data_len = data.size();

  if (type == libskiff::types::constant_type_e::STRING) {
    // Shave off string length as it wont be in memory at run time
    adt.loaded_const_address -= 8;
    data_len -= 8;
  }

  adt.constant_name_to_meta[name] = {
      .type = type, .address = current_address, .data_len = data_len};
  return true;
}

bool directive_string(const uint64_t line_number, const std::string &line,
                      assembler_data_t &adt)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  auto chunks = chunk_line(line);
  std::string location_info = "line" + std::to_string(line_number);

  if (chunks.size() != 3) {
    add_issue(location_info, "phase 3", "Malformed .string directive", adt,
              true);
    return false;
  }

  auto [okay, str] = get_string_literal(chunks[2]);

  if (!okay) {
    add_issue(location_info, "phase 3", "Invalid string literal: " + line, adt,
              true);
    return false;
  }

  auto value = adt.ins_generator.gen_string_constant(str);
  if (value == std::nullopt) {
    add_issue(location_info, "phase 3",
              "Unable to encode .string directive with value : " + chunks[2],
              adt, true);
    return false;
  }

  return add_constant(chunks[1], libskiff::types::constant_type_e::STRING,
                      *value, adt);
}

bool directive_float(const uint64_t line_number, const std::string &line,
                     assembler_data_t &adt)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  auto chunks = chunk_line(line);
  std::string location_info = "line" + std::to_string(line_number);

  if (chunks.size() != 3) {
    add_issue(location_info, "phase 3", "Malformed .float directive", adt,
              true);
    return false;
  }

  auto fp_value = get_number<double>(chunks[2]);

  if (fp_value == std::nullopt) {
    add_issue(location_info, "phase 3",
              "Unable to convert constant to double : " + chunks[2], adt, true);
    return false;
  }

  return add_constant(chunks[1], libskiff::types::constant_type_e::FLOAT,
                      adt.ins_generator.generate_fp_constant(*fp_value), adt);
}

template <class T>
bool directive_integer(const std::string &kind,
                       libskiff::types::constant_type_e type,
                       const uint64_t line_number, const std::string &line,
                       assembler_data_t &adt)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  auto chunks = chunk_line(line);
  std::string location_info = "line" + std::to_string(line_number);

  if (chunks.size() != 3) {
    add_issue(location_info, "phase 3", "Malformed " + kind + " directive", adt,
              true);
    return false;
  }

  auto value = get_number<T>(chunks[2]);
  if (value > std::numeric_limits<T>::max() ||
      value < std::numeric_limits<T>::min()) {
    add_issue(location_info, "phase 3",
              "Value out of range for " + kind + ": " + chunks[2], adt, true);
    return false;
  }

  if (value == std::nullopt) {
    add_issue(location_info, "phase 3",
              "Unable to confert constant value to type: " + chunks[2], adt,
              true);
    return false;
  }

  switch (type) {
  case libskiff::types::constant_type_e::I8:
    return add_constant(chunks[1], type,
                        adt.ins_generator.generate_i8_constant(*value), adt);
  case libskiff::types::constant_type_e::I16:
    return add_constant(chunks[1], type,
                        adt.ins_generator.generate_i16_constant(*value), adt);
  case libskiff::types::constant_type_e::I32:
    return add_constant(chunks[1], type,
                        adt.ins_generator.generate_i32_constant(*value), adt);
  case libskiff::types::constant_type_e::I64:
    return add_constant(chunks[1], type,
                        adt.ins_generator.generate_i64_constant(*value), adt);
  case libskiff::types::constant_type_e::U8:
    return add_constant(chunks[1], type,
                        adt.ins_generator.generate_u8_constant(*value), adt);
  case libskiff::types::constant_type_e::U16:
    return add_constant(chunks[1], type,
                        adt.ins_generator.generate_u16_constant(*value), adt);
  case libskiff::types::constant_type_e::U32:
    return add_constant(chunks[1], type,
                        adt.ins_generator.generate_u32_constant(*value), adt);
  case libskiff::types::constant_type_e::U64:
    return add_constant(chunks[1], type,
                        adt.ins_generator.generate_u64_constant(*value), adt);
  default:
    add_issue(location_info, "phase 3",
              "Internal error: Default hit while adding integer directive", adt,
              true);
    return false;
  }
}

bool build_nop(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 1) {
    add_issue(location_information, "phase 4", "Malformed NOP instruction", adt,
              true);
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_nop());
  return true;
}

bool build_exit(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 1) {
    add_issue(location_information, "phase 4", "Malformed EXIT instruction",
              adt, true);
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_exit());
  return true;
}

bool build_ret(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 1) {
    add_issue(location_information, "phase 4", "Malformed RET instruction", adt,
              true);
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_ret());
  return true;
}

bool build_eirq(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 1) {
    add_issue(location_information, "phase 4", "Malformed EIRQ instruction",
              adt, true);
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_eirq());
  return true;
}

bool build_dirq(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 1) {
    add_issue(location_information, "phase 4", "Malformed DIRQ instruction",
              adt, true);
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_dirq());
  return true;
}

std::tuple<bool, uint64_t, uint8_t, uint8_t>
validate_branch(std::string kind, const instruction_data_t &ins,
                assembler_data_t &adt)
{
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 4) {
    add_issue(location_information, "phase 4",
              "Malformed " + kind + " instruction", adt, true);
    return {false, 0, 0, 0};
  }

  auto address = get_label_address(ins.line_data.pieces[3], adt);
  if (address == std::nullopt) {
    add_issue(location_information, "phase 4",
              "Unknown label " + ins.line_data.pieces[3], adt, true);
    return {false, 0, 0, 0};
  }

  auto lhs = adt.ins_generator.get_register_value(ins.line_data.pieces[1]);
  if (lhs == std::nullopt) {
    add_issue(location_information, "phase 4",
              "Invalid register given to instruction", adt, true);
    return {false, 0, 0, 0};
  }

  auto rhs = adt.ins_generator.get_register_value(ins.line_data.pieces[2]);
  if (rhs == std::nullopt) {
    add_issue(location_information, "phase 4",
              "Invalid register given to instruction", adt, true);
    return {false, 0, 0, 0};
  }

  return {true, *address, *lhs, *rhs};
}

bool build_blt(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, address, lhs, rhs] = validate_branch("BLT", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_blt(lhs, rhs, address));
  return true;
}

bool build_bgt(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, address, lhs, rhs] = validate_branch("BGT", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_bgt(lhs, rhs, address));
  return true;
}

bool build_beq(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, address, lhs, rhs] = validate_branch("BEQ", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_beq(lhs, rhs, address));
  return true;
}

bool build_bltf(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, address, lhs, rhs] = validate_branch("BLTF", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_bltf(lhs, rhs, address));
  return true;
}

bool build_bgtf(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, address, lhs, rhs] = validate_branch("BGTF", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_bgtf(lhs, rhs, address));
  return true;
}

bool build_beqf(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, address, lhs, rhs] = validate_branch("BEQF", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_beqf(lhs, rhs, address));
  return true;
}

std::tuple<bool, uint64_t>
validate_address_holder(std::string kind, const instruction_data_t &ins,
                        assembler_data_t &adt)
{

  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 2) {
    add_issue(location_information, "phase 4",
              "Malformed " + kind + " instruction", adt, true);
    return {false, 0};
  }

  auto address = get_label_address(ins.line_data.pieces[1], adt);
  if (address == std::nullopt) {
    add_issue(location_information, "phase 4",
              "Unknown label given to " + kind +
                  " instruction: " + ins.line_data.pieces[1],
              adt, true);
    return {false, 0};
  }

  return {true, *address};
}

bool build_jmp(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, address] = validate_address_holder("JMP", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_jmp(address));
  return true;
}

bool build_call(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, address] = validate_address_holder("CALL", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_call(address));
  return true;
}

std::optional<uint64_t> get_const_value(const uint64_t line_number,
                                        const std::string item,
                                        assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  std::string location_information = "line " + std::to_string(line_number);

  if (item.size() < 2) {
    return std::nullopt;
  }

  // Check if its a raw value
  if (item.starts_with('@')) {
    std::string s = item.substr(1, item.length());
    if (s.starts_with('-')) {
      auto v = get_number<int64_t>(s);
      if (v != std::nullopt) {
        return *v;
      }
      return std::nullopt;
    }
    else {
      return get_number<uint64_t>(s);
    }
  }

  // Check if its an address
  if (item.starts_with('&')) {
    std::string s = item.substr(1, item.length());

    // Check labels
    auto label_address = get_label_address(s, adt);
    if (label_address != std::nullopt) {
      if (label_address > std::numeric_limits<uint64_t>::max()) {
        add_issue(location_information, "phase 4",
                  "Requested mov of label address whose value exceeds that "
                  "able to be stored in a uint64_t",
                  adt, true);
        return std::nullopt;
      }
      return *label_address;
    }

    // Check constants
    if (adt.constant_name_to_meta.find(s) != adt.constant_name_to_meta.end()) {
      auto constant = adt.constant_name_to_meta[s];
      if (constant.address > std::numeric_limits<uint64_t>::max()) {
        add_issue(location_information, "phase 4",
                  "Requested mov of constant address whose value exceeds that "
                  "able to be stored in a uint64_t",
                  adt, true);
        return std::nullopt;
      }
      return constant.address;
    }
    return std::nullopt;
  }

  // Check if its a length
  if (item.starts_with('#')) {
    std::string s = item.substr(1, item.length());

    // Check constants
    if (adt.constant_name_to_meta.find(s) != adt.constant_name_to_meta.end()) {
      auto constant = adt.constant_name_to_meta[s];

      auto length = static_cast<uint64_t>(constant.data_len);
      if (length > std::numeric_limits<uint64_t>::max()) {
        add_issue(location_information, "phase 4",
                  "Requested mov of constant whose length exceeds that "
                  "able to be stored in a uint64_t",
                  adt, true);
        return std::nullopt;
      }
      return length;
    }
  }

  return std::nullopt;
}

bool build_mov(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 3) {
    add_issue(location_information, "phase 4", "Malformed MOV instruction", adt,
              true);
    return false;
  }

  auto lhs = adt.ins_generator.get_register_value(ins.line_data.pieces[1]);
  if (lhs == std::nullopt) {
    add_issue(location_information, "phase 4", "Invalid register", adt, true);
    return false;
  }

  auto value =
      get_const_value(ins.line_data.line_number, ins.line_data.pieces[2], adt);
  if (value == std::nullopt) {
    add_issue(location_information, "phase 4",
              "Unable to retrieve source for MOV instruction", adt, true);
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_mov(*lhs, *value));
  return true;
}

std::tuple<bool, uint8_t, uint8_t, uint8_t>
validate_arithmetic(std::string kind, const instruction_data_t &ins,
                    assembler_data_t &adt)
{
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 4) {
    add_issue(location_information, "phase 4",
              "Malformed " + kind + " instruction", adt, true);
    return {false, 0, 0, 0};
  }

  auto dest = adt.ins_generator.get_register_value(ins.line_data.pieces[1]);
  if (dest == std::nullopt) {
    add_issue(location_information, "phase 4",
              "Invalid register given to instruction", adt, true);
    return {false, 0, 0, 0};
  }

  auto lhs = adt.ins_generator.get_register_value(ins.line_data.pieces[2]);
  if (lhs == std::nullopt) {
    add_issue(location_information, "phase 4",
              "Invalid register given to instruction", adt, true);
    return {false, 0, 0, 0};
  }

  auto rhs = adt.ins_generator.get_register_value(ins.line_data.pieces[3]);
  if (rhs == std::nullopt) {
    add_issue(location_information, "phase 4",
              "Invalid register given to instruction", adt, true);
    return {false, 0, 0, 0};
  }

  return {true, *dest, *lhs, *rhs};
}

bool build_add(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("ADD", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_add(dest, lhs, rhs));
  return true;
}

bool build_sub(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("SUB", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_sub(dest, lhs, rhs));
  return true;
}

bool build_div(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("DIV", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_div(dest, lhs, rhs));
  return true;
}

bool build_mul(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("MUL", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_mul(dest, lhs, rhs));
  return true;
}

bool build_addf(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("ADDF", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_addf(dest, lhs, rhs));
  return true;
}

bool build_subf(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("SUBF", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_subf(dest, lhs, rhs));
  return true;
}

bool build_divf(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("DIVF", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_divf(dest, lhs, rhs));
  return true;
}

bool build_mulf(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("MULF", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_mulf(dest, lhs, rhs));
  return true;
}

bool build_lsh(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("LSH", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_lsh(dest, lhs, rhs));
  return true;
}

bool build_rsh(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("RSH", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_rsh(dest, lhs, rhs));
  return true;
}

bool build_and(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("AND", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_and(dest, lhs, rhs));
  return true;
}

bool build_or(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("OR", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_or(dest, lhs, rhs));
  return true;
}

bool build_xor(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  auto [success, dest, lhs, rhs] = validate_arithmetic("XOR", ins, adt);
  if (!success) {
    return false;
  }

  adt.bin_generator.add_instruction(adt.ins_generator.gen_xor(dest, lhs, rhs));
  return true;
}

std::tuple<bool, uint8_t, uint8_t>
validate_two_reg_instruction(std::string kind, const instruction_data_t &ins,
                             assembler_data_t &adt)
{
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 3) {
    add_issue(location_information, "phase 4",
              "Malformed " + kind + " instruction", adt, true);
    return {false, 0, 0};
  }

  auto lhs = adt.ins_generator.get_register_value(ins.line_data.pieces[1]);
  if (lhs == std::nullopt) {
    add_issue(location_information, "phase 4", "Invalid lhs register", adt,
              true);
    return {false, 0, 0};
  }

  auto rhs = adt.ins_generator.get_register_value(ins.line_data.pieces[2]);
  if (rhs == std::nullopt) {
    add_issue(location_information, "phase 4", "Invalid rhs register", adt,
              true);
    return {false, 0, 0};
  }

  return {true, *lhs, *rhs};
}

bool build_not(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, dest, source] = validate_two_reg_instruction("NOT", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_not(dest, source));
  return true;
}

bool build_aseq(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, lhs, rhs] = validate_two_reg_instruction("ASEQ", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_aseq(lhs, rhs));
  return true;
}

bool build_asne(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, lhs, rhs] = validate_two_reg_instruction("ASNE", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_asne(lhs, rhs));
  return true;
}

bool build_alloc(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, lhs, rhs] = validate_two_reg_instruction("ALLOC", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_alloc(lhs, rhs));
  return true;
}

std::tuple<bool, uint64_t> validate_raw_singleton_instruction(
    std::string kind, const instruction_data_t &ins, assembler_data_t &adt)
{
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 2) {
    add_issue(location_information, "phase 4",
              "Malformed " + kind + " instruction", adt, true);
    return {false, 0};
  }

  auto value = get_number<uint64_t>(ins.line_data.pieces[1]);

  if (value == std::nullopt) {
    add_issue(location_information, "phase 4",
              "Invalid number given to " + kind, adt, true);
    return {false, 0};
  }

  return {true, *value};
}

bool build_syscall(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, val] = validate_raw_singleton_instruction("SYSCALL", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_syscall(val));
  return true;
}

bool build_debug(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, val] = validate_raw_singleton_instruction("DEBUG", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_debug(val));
  return true;
}

std::tuple<bool, uint8_t>
validate_one_reg_instruction(std::string kind, const instruction_data_t &ins,
                             assembler_data_t &adt)
{
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 2) {
    add_issue(location_information, "phase 4",
              "Malformed " + kind + " instruction", adt, true);
    return {false, 0};
  }

  auto value = adt.ins_generator.get_register_value(ins.line_data.pieces[1]);
  if (value == std::nullopt) {
    add_issue(location_information, "phase 4", "Invalid register", adt, true);
    return {false, 0};
  }

  return {true, *value};
}

bool build_free(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, val] = validate_one_reg_instruction("FREE", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_free(val));
  return true;
}

bool build_push_w(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, val] = validate_one_reg_instruction("PUSH_W", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_push_w(val));
  return true;
}

bool build_push_dw(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, val] = validate_one_reg_instruction("PUSH_DW", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_push_dw(val));
  return true;
}

bool build_push_qw(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, val] = validate_one_reg_instruction("PUSH_QW", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_push_qw(val));
  return true;
}

bool build_pop_w(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, val] = validate_one_reg_instruction("POP_W", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_pop_w(val));
  return true;
}

bool build_pop_dw(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, val] = validate_one_reg_instruction("POP_DW", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_pop_dw(val));
  return true;
}

bool build_pop_qw(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, val] = validate_one_reg_instruction("POP_QW", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(adt.ins_generator.gen_pop_qw(val));
  return true;
}

std::tuple<bool, uint8_t, uint8_t, uint8_t>
validate_three_reg_instruction(std::string kind, const instruction_data_t &ins,
                               assembler_data_t &adt)
{
  std::string location_information =
      "line " + std::to_string(ins.line_data.line_number);

  if (ins.line_data.pieces.size() != 4) {
    add_issue(location_information, "phase 4",
              "Malformed " + kind + " instruction", adt, true);
    return {false, 0, 0, 0};
  }

  auto first = adt.ins_generator.get_register_value(ins.line_data.pieces[1]);
  if (first == std::nullopt) {
    add_issue(location_information, "phase 4", "Invalid lhs register", adt,
              true);
    return {false, 0, 0, 0};
  }

  auto second = adt.ins_generator.get_register_value(ins.line_data.pieces[2]);
  if (second == std::nullopt) {
    add_issue(location_information, "phase 4", "Invalid rhs register", adt,
              true);
    return {false, 0, 0, 0};
  }

  auto third = adt.ins_generator.get_register_value(ins.line_data.pieces[3]);
  if (third == std::nullopt) {
    add_issue(location_information, "phase 4", "Invalid rhs register", adt,
              true);
    return {false, 0, 0, 0};
  }

  return {true, *first, *second, *third};
}

bool build_store_w(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, idx, offset, data] =
      validate_three_reg_instruction("STORE_W", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_store_word(idx, offset, data));
  return true;
}

bool build_store_dw(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, idx, offset, data] =
      validate_three_reg_instruction("STORE_DW", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_store_dword(idx, offset, data));
  return true;
}

bool build_store_qw(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, idx, offset, data] =
      validate_three_reg_instruction("STORE_QW", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_store_qword(idx, offset, data));
  return true;
}

bool build_load_w(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, idx, offset, data] =
      validate_three_reg_instruction("LOAD_W", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_load_word(idx, offset, data));
  return true;
}

bool build_load_dw(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, idx, offset, data] =
      validate_three_reg_instruction("LOAD_DW", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_load_dword(idx, offset, data));
  return true;
}

bool build_load_qw(const instruction_data_t &ins, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";
  auto [success, idx, offset, data] =
      validate_three_reg_instruction("LOAD_QW", ins, adt);
  if (!success) {
    return false;
  }
  adt.bin_generator.add_instruction(
      adt.ins_generator.gen_load_qword(idx, offset, data));
  return true;
}

// 1) Iterate over file looking for macros and dropping them in where they
// belong.
//    This stage will also remove all comments and blank lines
//
//    Returns status bool with a vector of pairs containing line number and line
//    data.
//
std::tuple<bool, std::vector<std::tuple<uint64_t, std::string>>>
phase_1(const std::string &input, assembler_data_t &adt)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  std::ifstream ifs(input);
  if (!ifs.is_open()) {
    add_issue({}, "phase 1", "Unable to open file for assembly", adt, true);
    return {false, {}};
  }

  std::vector<std::tuple<uint64_t, std::string>> raw_file_data_no_comments;

  uint64_t line_no{0};
  std::string line;
  while (std::getline(ifs, line)) {
    line_no++;

    // Remove comments
    std::string stripped_line = line.substr(0, line.find_first_of(';'));

    // If there is nothing of worth left, remove the item
    if (stripped_line.empty() ||
        std::all_of(stripped_line.begin(), stripped_line.end(), isspace)) {
      continue;
    }
    raw_file_data_no_comments.push_back(std::make_pair(line_no, stripped_line));
  }
  ifs.close();

  // Scan for macros
  uint64_t item_count{0};
  std::unordered_map<std::string, std::vector<std::string>> macro_to_contents;

  for (auto i = 0; i < raw_file_data_no_comments.size(); i++) {
    auto [line_number, line_data] = raw_file_data_no_comments[i];
    std::string location_info = "line " + std::to_string(line_number);

    if (line_data.rfind("#macro", 0) == 0) {

      auto chunks = chunk_line(line_data);

      if (chunks.size() < 3 || chunks.size() > 4) {
        add_issue(location_info, "phase 1", "Malformed macro : " + chunks[0],
                  adt, true);
        return {false, {}};
      }

      auto [valid, line] = get_string_literal(chunks[2]);

      if (!valid) {
        add_issue(location_info, "phase 1",
                  "Malformed macro : " + chunks[0] + ", Invalid string literal",
                  adt, true);
      }

      std::vector<std::string> data = {line};

      // check to see if its multi-lined
      if (chunks.size() == 4) {
        if (chunks[3] != "\\") {
          add_issue(
              location_info, "phase 1",
              "Malformed macro : " + chunks[0] +
                  ", Expected '\\' as last piece to macro line of given length",
              adt, true);
          return {false, {}};
        }

        bool done = false;
        while (!done) {

          // EOF
          if (i == raw_file_data_no_comments.size() - 1) {
            done = true;
          }

          // Go to next item in file
          i += 1;

          // Chunk the next line
          auto next = chunk_line(std::get<1>(raw_file_data_no_comments[i]));

          if (next.size() > 2) {

            add_issue(location_info, "phase 1",
                      "Malformed macro : " + chunks[0] +
                          ", Continuation malformation",
                      adt, true);
            return {false, {}};
          }

          auto [is_valid, next_lit] = get_string_literal(next[0]);

          if (!valid) {
            add_issue(location_info, "phase 1",
                      "Malformed macro : " + chunks[1] +
                          ", Invalid string literal",
                      adt, true);
            return {false, {}};
          }

          data.push_back(next_lit);

          if (next.size() == 2) {
            if (next[1] != "\\") {
              add_issue(
                  location_info, "phase 1",
                  "Malformed macro : " + chunks[0] +
                      ", Expected '\\' as last piece to macro line of given "
                      "length",
                  adt, true);
              return {false, {}};
            }
          }
          else {
            done = true;
          }
        }
      }

      // Add to macro map
      macro_to_contents[chunks[1]] = data;
      item_count = data.size();
    }
  }

  std::vector<std::tuple<uint64_t, std::string>> full_file_data;
  full_file_data.reserve(raw_file_data_no_comments.size() + item_count);

  // Insert the macros
  uint64_t added_lines{0};
  for (auto &line_data : raw_file_data_no_comments) {
    auto [num, line] = line_data;

    // Skip the actual macro definitions
    if (line.rfind("#macro", 0) == 0) {
      continue;
    }

    auto c_line_chunks = chunk_line(line);
    if (c_line_chunks.size() == 1 && c_line_chunks[0].starts_with('#') &&
        c_line_chunks[0].size() > 1) {
      auto actual_macro_name =
          c_line_chunks[0].substr(1, c_line_chunks[0].size());
      if (macro_to_contents.find(actual_macro_name) ==
          macro_to_contents.end()) {

        // For this error we want the regular line number so they can locate the
        // goof
        add_issue("line" + std::to_string(num), "phase 1", "Unknown macro", adt,
                  true);
        return {false, {}};
      }
      auto macro_contents = macro_to_contents[actual_macro_name];
      for (auto &macro_line : macro_contents) {

        // Add the number of lines we've inserted to the line num
        full_file_data.push_back(
            std::make_pair(num + added_lines++, macro_line));
      }

      // One line was replaced so we sub it
      added_lines -= 1;
      continue;
    }

    full_file_data.push_back(std::make_pair(num + added_lines, line));
  }

  return {true, full_file_data};
}

//  2) Enumerate instructions and generate the label data along with pre-parsed
//  instruction data
//
bool phase_2(std::vector<std::tuple<uint64_t, std::string>> &raw_file_data,
             assembler_data_t &adt)
{
  auto string_to_instruction_map = get_string_to_instruction_map();
  auto instruction_to_size_map =
      libskiff::bytecode::instructions::get_instruction_to_size_map();

  adt.code_found = false;
  uint64_t num_instructions{0};
  for (uint64_t i = 0; i < raw_file_data.size(); i++) {

    auto [line_number, line] = raw_file_data[i];
    auto chunks = chunk_line(line);

    std::string location_information = "line" + std::to_string(line_number);

    if (chunks[0] == ".code") {
      if (adt.code_found) {
        add_issue(location_information, "phase 2", "Duplicate .code directive",
                  adt, true);
        return false;
      }
      if (chunks.size() > 1) {
        add_issue(location_information, "phase 2", "Malformed .code directive",
                  adt, true);
        return false;
      }
      adt.code_found = true;
      continue;
    }

    if (!adt.code_found) {

      //  Add raw directives for processing later
      //
      if (line.starts_with('.')) {
        adt.raw_directives.push_back(raw_file_data[i]);
      }
      continue;
    }

    // Check for label
    //
    if (std::regex_match(chunks[0], std::regex("^[a-zA-Z0-9_]+:$"))) {
      if (chunks.size() != 1) {
        add_issue(location_information, "phase 2", "Malformed Label", adt,
                  true);
        return false;
      }

      std::string label_name =
          chunks[0].substr(0, chunks[0].find_first_of(':'));

      //  Check to see if its an interrupt label
      //
      if (std::regex_match(label_name, std::regex("^interrupt_[0-9]+"))) {

        auto value =
            get_number<uint64_t>(label_name.substr(10, label_name.size()));

        if (value == std::nullopt) {
          add_issue(location_information, "phase 2",
                    "Invalid number for given for interrupt", adt, true);
          return false;
        }

        if (adt.interrupts.contains(*value)) {
          add_issue(location_information, "phase 2",
                    "Duplicate interrupt number : " + std::to_string(*value),
                    adt, true);
          return false;
        }

        LOG(DEBUG) << TAG("assembler") << "Interrupt [" << *value
                   << "] mapped to instruction number [" << num_instructions
                   << "]\n";

        adt.bin_generator.add_interrupt(
            adt.ins_generator.gen_interrupt_table_entry(*value,
                                                        num_instructions));
        adt.interrupts.insert(*value);
      }

      if (adt.label_to_instruction_address.find(label_name) !=
          adt.label_to_instruction_address.end()) {
        add_issue(location_information, "phase 2",
                  "Duplicate label name " + label_name, adt, true);
        return false;
      }

      adt.label_to_instruction_address[label_name] = num_instructions;

      LOG(DEBUG) << TAG("assembler") << "Added lable `" << label_name
                 << "` at instruction location [" << num_instructions << "]\n";
      continue;
    } // End label scan

    // If we get here it better be an instruction..
    if (chunks.empty()) {
      add_issue(location_information, "phase 2",
                "Internal error : Somehow chunked an empty line", adt, true);
      return false;
    }

    if (string_to_instruction_map.find(chunks[0]) ==
        string_to_instruction_map.end()) {
      add_issue(location_information, "phase 2",
                "Unrecognized instruction" + chunks[0], adt, true);
      return false;
    }

    auto instruction = string_to_instruction_map[chunks[0]];

    num_instructions += 1;

    // Add the information for parsing the instruction in the next phase
    adt.instructions_to_parse.push_back(
        {.instruction_length = instruction_to_size_map[instruction],
         .instruction_id = instruction,
         .line_data = {.line_number = line_number, .pieces = chunks}});
  }
  return true;
}

//  3) Resolve directives
//
bool phase_3(assembler_data_t &adt)
{
  struct std_opt_t {
    std::string directive;
    std::function<bool(const uint64_t, const std::string &, assembler_data_t &)>
        func;
  };

  std::vector<std_opt_t> valid_std_directives = {
      {".init", directive_init},
      {".float", directive_float},
      {".string", directive_string},
      {".debug", directive_debug},
  };

  struct int_opt_t {
    std::string directive;
    libskiff::types::constant_type_e type;
    std::function<bool(const std::string &, libskiff::types::constant_type_e,
                       const uint64_t, const std::string &, assembler_data_t &)>
        func;
  };

  std::vector<int_opt_t> valid_integer_directives = {
      {".i8", libskiff::types::constant_type_e::I8, directive_integer<int16_t>},
      {".i16", libskiff::types::constant_type_e::I16,
       directive_integer<int16_t>},
      {".i32", libskiff::types::constant_type_e::I32,
       directive_integer<int32_t>},
      {".i64", libskiff::types::constant_type_e::I64,
       directive_integer<int64_t>},
      {".u8", libskiff::types::constant_type_e::U8,
       directive_integer<uint16_t>},
      {".u16", libskiff::types::constant_type_e::U16,
       directive_integer<uint16_t>},
      {".u32", libskiff::types::constant_type_e::U32,
       directive_integer<uint32_t>},
      {".u64", libskiff::types::constant_type_e::U64,
       directive_integer<uint64_t>}};

  uint64_t directives{1};
  for (auto [line_number, line] : adt.raw_directives) {
    bool found{false};
    for (auto &&entry : valid_std_directives) {
      if (line.starts_with(entry.directive.c_str())) {
        if (!entry.func(line_number, line, adt)) {
          return false;
        }
        found = true;
      }
    }

    for (auto &&entry : valid_integer_directives) {
      if (line.starts_with(entry.directive.c_str())) {
        if (!entry.func(entry.directive, entry.type, line_number, line, adt)) {
          return false;
        }
        found = true;
      }
    }

    if (!found) {
      add_issue("line" + std::to_string(line_number), "phase 3",
                "Unknown directive: " + line, adt, true);
      return false;
    }
  }
  return true;
}

//  4) Generate instructios
//
bool phase_4(assembler_data_t &adt)
{
  struct ins_build_entry_t {
    std::string ins;
    std::function<bool(const instruction_data_t &ins, assembler_data_t &adt)>
        func;
  };

  std::vector<ins_build_entry_t> ins_build_lit = {
      {"nop", build_nop},         {"exit", build_exit},
      {"ret", build_ret},         {"eirq", build_eirq},
      {"dirq", build_dirq},       {"blt", build_blt},
      {"bgt", build_bgt},         {"beq", build_beq},
      {"bltf", build_bltf},       {"bgtf", build_bgtf},
      {"beqf", build_beqf},       {"call", build_call},
      {"jmp", build_jmp},         {"mov", build_mov},
      {"add", build_add},         {"sub", build_sub},
      {"div", build_div},         {"mul", build_mul},
      {"addf", build_addf},       {"subf", build_subf},
      {"divf", build_divf},       {"mulf", build_mulf},
      {"lsh", build_lsh},         {"rsh", build_rsh},
      {"and", build_and},         {"or", build_or},
      {"xor", build_xor},         {"not", build_not},
      {"aseq", build_aseq},       {"asne", build_asne},
      {"syscall", build_syscall}, {"debug", build_debug},
      {"pop_w", build_pop_w},     {"pop_dw", build_pop_dw},
      {"pop_qw", build_pop_qw},   {"push_w", build_push_w},
      {"push_dw", build_push_dw}, {"push_qw", build_push_qw},
      {"alloc", build_alloc},     {"free", build_free},
      {"sw", build_store_w},      {"sdw", build_store_dw},
      {"sqw", build_store_qw},    {"lw", build_load_w},
      {"ldw", build_load_dw},     {"lqw", build_load_qw},
  };

  /*
    NOTE:
      Now that this is all instructions, a potential speedup would be to divide
      `adt.instructions_to_parse` into some number of chunks (3-4) and
    parallelize their generation. This would mean having to make the generators
    thread safe or parallel safe... however it would be implemented, but here
    all things are resolved and its just grunt work generating bytes
  */

  for (auto &&item : adt.instructions_to_parse) {

    std::string location_information =
        "line " + std::to_string(item.line_data.line_number);

    // This shouldn't happen, but check anyway
    if (item.line_data.pieces.empty()) {
      add_issue(location_information, "phase 4", "Internal Error: Empty pieces",
                adt, true);
      return false;
    }

    bool found{false};
    for (auto &&entry : ins_build_lit) {
      if (item.line_data.pieces[0] == entry.ins) {
        if (!entry.func(item, adt)) {
          return false;
        }
        found = true;
      }
    }
    if (!found) {
      add_issue(location_information, "phase 4",
                "Unknown instruction: " + item.line_data.pieces[0], adt, true);
      return false;
    }
  }
  return true;
}

assembled_t assemble(const std::string &input)
{
  LOG(DEBUG) << TAG("assembler:func") << __func__ << "\n";

  assembler_data_t adt;

  // Retrieve file data (pair: line_number, data) with macros resolved
  //
  auto [phase_one_success, file_data] = phase_1(input, adt);

  if (!phase_one_success) {
    LOG(FATAL) << TAG("assembler") << "phase 1 failed\n";
    return adt.result;
  }

  //  Resolve label information and setup instructions/directives to be parsed
  //
  if (!phase_2(file_data, adt)) {
    LOG(FATAL) << TAG("assembler") << "phase 2 failed\n";
    return adt.result;
  }

  /*
    NOTE: Phase 3 and phase 4 _could_ be done completly in parallel if we
          for some reason needed to speed thing sup
  */

  //  Parse directives
  //
  if (!phase_3(adt)) {
    LOG(FATAL) << TAG("assembler") << "phase 3 failed\n";
    return adt.result;
  }

  //  Parse instructions
  //
  if (!phase_4(adt)) {
    LOG(FATAL) << TAG("assembler") << "phase 4 failed\n";
    return adt.result;
  }

  if (!adt.code_found) {
    LOG(FATAL) << TAG("assembler") << ".code directive was not present\n";
  }
  if (!adt.init_found) {
    LOG(FATAL) << TAG("assembler") << ".init directive was not present\n";
  }

  LOG(TRACE) << TAG("assembler") << "Building sections\n";
  for (auto &item : adt.label_to_instruction_address) {
    auto section = adt.ins_generator.gen_lib_section(item.second, item.first);
    if (section == std::nullopt) {
      LOG(FATAL) << TAG("assembler") << "Failed to generate section\n";
      return adt.result;
    }

    adt.bin_generator.add_section(*section);
  }

  adt.result.bin = adt.bin_generator.generate_binary();
  adt.result.stats.num_instructions =
      adt.ins_generator.get_number_instructions_generated();
  return adt.result;
}

} // namespace assembler
} // namespace libskiff
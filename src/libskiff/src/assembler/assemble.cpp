/*
  As of now (3-30-22) assembler is meant to be the minimum viable assembler.
  The intent is to get something that can implement the instruction
  set for the sake of creating the libskiff instruction_generator
  and generating first stage binaries for VM development.

  Once the instruction set is solidified into 1.0, this assembler could
  be revisited to be optimized, or it could be left as is...

  Once the instruction set hits 1.0 a higher level language is planned
  to be developed, so this assembler might then be removed entirely.
*/

#include "libskiff/assembler/assemble.hpp"

#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <unordered_map>

#include "libskiff/generators/binary_generator.hpp"
#include "libskiff/generators/instruction_generator.hpp"
#include "libskiff/logging/aixlog.hpp"
#include "libskiff/types.hpp"

namespace libskiff {
namespace assembler {
namespace {

template <class T> std::optional<T> get_number(const std::string value)
{
  T value_out = 0;
  std::stringstream ss(value);
  ss >> value_out;
  if (ss.fail()) {
    return std::nullopt;
  }
  return value_out;
}

class assembler_c {
public:
  assembler_c(const std::string &input);
  ~assembler_c() = default;
  assembled_t get_result();
  void assemble();

private:
  static constexpr uint8_t INSTRUCTION_SIZE_BYTES = 8;

  enum class state_e {
    DIRECTIVE_DIGESTION,
    INSTRUCTION_DIGESTION,
  };

  struct constant_value_t {
    libskiff::types::constant_type_e type;
    uint64_t address;
    std::vector<uint8_t> data;
  };

  struct match_t {
    std::regex reg;
    std::function<bool()> call;
  };

  struct directive_checks_t {
    bool init;
    bool code;
    bool debug;
    bool data_first;
  };

  const std::string &_input_file;
  uint64_t _current_line_number{0};
  uint64_t _current_processed_line_number{0};
  state_e _state{state_e::DIRECTIVE_DIGESTION};
  assembled_t _result{.stats = {.num_instructions = 0},
                      .errors = std::nullopt,
                      .warnings = std::nullopt,
                      .bin = std::nullopt};
  std::vector<std::string> _file_data;
  std::vector<match_t> _directive_match;
  std::vector<match_t> _instruction_match;

  directive_checks_t _directive_checks = {false, false, false, false};
  std::unordered_map<std::string, uint64_t> _label_to_location;
  std::unordered_map<std::string, constant_value_t> _constant_name_to_value;
  uint64_t _expected_bin_size{0};
  std::vector<std::string> _current_chunks;
  libskiff::instructions::instruction_generator_c _ins_gen;
  std::unique_ptr<libskiff::generator::binary_generator> _generator;

  std::string remove_comments(const std::string &str);
  void add_error(const std::string &str);
  void add_warning(const std::string &str);
  void add_debug(const std::string &str);
  void add_trace(const std::string &str);
  void pre_scan();
  void parse(const std::vector<match_t> &function_match);

  bool add_constant(std::string name, libskiff::types::constant_type_e type,
                    std::vector<uint8_t> value);
  bool directive_init();
  bool directive_lib();
  bool directive_debug();
  bool directive_code();
  bool directive_string();
  bool directive_float();
  bool directive_int_8();
  bool directive_int_16();
  bool directive_int_32();
  bool directive_int_64();
  bool directive_uint_8();
  bool directive_uint_16();
  bool directive_uint_32();
  bool directive_uint_64();

  void add_instruction_bytes(std::vector<uint8_t> bytes);
  std::optional<uint64_t> get_label_address(const std::string label);
  std::optional<uint32_t> get_value(const std::string item);

  bool build_nop();
  bool build_exit();
  bool build_blt();
  bool build_bgt();
  bool build_beq();
  bool build_jmp();
  bool build_call();
  bool build_ret();
  bool build_mov();
  bool build_add();
  bool build_sub();
  bool build_div();
  bool build_mul();
  bool build_addf();
  bool build_subf();
  bool build_divf();
  bool build_mulf();
  bool build_lsh();
  bool build_rsh();
  bool build_and();
  bool build_or();
  bool build_xor();
  bool build_not();
  bool build_bltf();
  bool build_bgtf();
  bool build_beqf();
  bool build_asne();
  bool build_aseq();

  bool build_push_w();
  bool build_push_dw();
  bool build_push_qw();
  bool build_pop_w();
  bool build_pop_dw();
  bool build_pop_qw();
};

inline std::vector<std::string> chunk_line(std::string line)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

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
} // namespace

assembler_c::assembler_c(const std::string &input) : _input_file(input)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  _directive_match = {
      match_t{
          std::regex("^\\.init$"),
          std::bind(&libskiff::assembler::assembler_c::directive_init, this)},
      match_t{
          std::regex("^\\.debug$"),
          std::bind(&libskiff::assembler::assembler_c::directive_debug, this)},
      match_t{
          std::regex("^\\.code$"),
          std::bind(&libskiff::assembler::assembler_c::directive_code, this)},
      match_t{
          std::regex("^\\.string$"),
          std::bind(&libskiff::assembler::assembler_c::directive_string, this)},
      match_t{
          std::regex("^\\.float$"),
          std::bind(&libskiff::assembler::assembler_c::directive_float, this)},
      match_t{
          std::regex("^\\.i8$"),
          std::bind(&libskiff::assembler::assembler_c::directive_int_8, this)},
      match_t{
          std::regex("^\\.i16$"),
          std::bind(&libskiff::assembler::assembler_c::directive_int_16, this)},
      match_t{
          std::regex("^\\.i32$"),
          std::bind(&libskiff::assembler::assembler_c::directive_int_32, this)},
      match_t{
          std::regex("^\\.i64$"),
          std::bind(&libskiff::assembler::assembler_c::directive_int_64, this)},
      match_t{
          std::regex("^\\.u8$"),
          std::bind(&libskiff::assembler::assembler_c::directive_uint_8, this)},
      match_t{std::regex("^\\.u16$"),
              std::bind(&libskiff::assembler::assembler_c::directive_uint_16,
                        this)},
      match_t{std::regex("^\\.u32$"),
              std::bind(&libskiff::assembler::assembler_c::directive_uint_32,
                        this)},
      match_t{std::regex("^\\.u64$"),
              std::bind(&libskiff::assembler::assembler_c::directive_uint_64,
                        this)},
  };
  _instruction_match = {
      match_t{std::regex("^nop$"),
              std::bind(&libskiff::assembler::assembler_c::build_nop, this)},
      match_t{std::regex("^blt"),
              std::bind(&libskiff::assembler::assembler_c::build_blt, this)},
      match_t{std::regex("^bgt"),
              std::bind(&libskiff::assembler::assembler_c::build_bgt, this)},
      match_t{std::regex("^beq"),
              std::bind(&libskiff::assembler::assembler_c::build_beq, this)},
      match_t{std::regex("^jmp"),
              std::bind(&libskiff::assembler::assembler_c::build_jmp, this)},
      match_t{std::regex("^call"),
              std::bind(&libskiff::assembler::assembler_c::build_call, this)},
      match_t{std::regex("^ret"),
              std::bind(&libskiff::assembler::assembler_c::build_ret, this)},
      match_t{std::regex("^exit"),
              std::bind(&libskiff::assembler::assembler_c::build_exit, this)},
      match_t{std::regex("^mov"),
              std::bind(&libskiff::assembler::assembler_c::build_mov, this)},
      match_t{std::regex("^add"),
              std::bind(&libskiff::assembler::assembler_c::build_add, this)},
      match_t{std::regex("^sub"),
              std::bind(&libskiff::assembler::assembler_c::build_sub, this)},
      match_t{std::regex("^div"),
              std::bind(&libskiff::assembler::assembler_c::build_div, this)},
      match_t{std::regex("^mul"),
              std::bind(&libskiff::assembler::assembler_c::build_mul, this)},
      match_t{std::regex("^addf"),
              std::bind(&libskiff::assembler::assembler_c::build_addf, this)},
      match_t{std::regex("^subf"),
              std::bind(&libskiff::assembler::assembler_c::build_subf, this)},
      match_t{std::regex("^divf"),
              std::bind(&libskiff::assembler::assembler_c::build_divf, this)},
      match_t{std::regex("^mulf"),
              std::bind(&libskiff::assembler::assembler_c::build_mulf, this)},
      match_t{std::regex("^lsh"),
              std::bind(&libskiff::assembler::assembler_c::build_lsh, this)},
      match_t{std::regex("^rsh"),
              std::bind(&libskiff::assembler::assembler_c::build_rsh, this)},
      match_t{std::regex("^and"),
              std::bind(&libskiff::assembler::assembler_c::build_and, this)},
      match_t{std::regex("^or"),
              std::bind(&libskiff::assembler::assembler_c::build_or, this)},
      match_t{std::regex("^xor"),
              std::bind(&libskiff::assembler::assembler_c::build_xor, this)},
      match_t{std::regex("^not"),
              std::bind(&libskiff::assembler::assembler_c::build_not, this)},
      match_t{std::regex("^bltf"),
              std::bind(&libskiff::assembler::assembler_c::build_bltf, this)},
      match_t{std::regex("^bgtf"),
              std::bind(&libskiff::assembler::assembler_c::build_bgtf, this)},
      match_t{std::regex("^beqf"),
              std::bind(&libskiff::assembler::assembler_c::build_beqf, this)},
      match_t{std::regex("^asne"),
              std::bind(&libskiff::assembler::assembler_c::build_asne, this)},
      match_t{std::regex("^aseq"),
              std::bind(&libskiff::assembler::assembler_c::build_aseq, this)},
      match_t{std::regex("^push_w"),
              std::bind(&libskiff::assembler::assembler_c::build_push_w, this)},
      match_t{std::regex("^push_dw"),
              std::bind(&libskiff::assembler::assembler_c::build_push_dw, this)},
      match_t{std::regex("^push_qw"),
              std::bind(&libskiff::assembler::assembler_c::build_push_qw, this)},
      match_t{std::regex("^pop_w"),
              std::bind(&libskiff::assembler::assembler_c::build_pop_w, this)},
      match_t{std::regex("^pop_dw"),
              std::bind(&libskiff::assembler::assembler_c::build_pop_dw, this)},
      match_t{std::regex("^pop_qw"),
              std::bind(&libskiff::assembler::assembler_c::build_pop_qw, this)},
  };
}

assembled_t assembler_c::get_result()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  // Something went wrong and _generator didn't get defined
  if (!_generator) {
    return {.stats = {.num_instructions = 0},
            .errors = std::nullopt,
            .warnings = std::nullopt,
            .bin = std::nullopt};
  }

  LOG(TRACE) << TAG("assembler") << "Building sections\n";
  for (auto &item : _label_to_location) {
    auto section = _ins_gen.gen_lib_section(item.second, item.first);
    if (section == std::nullopt) {
      add_error("Unable to encode section");
    }
    _generator->add_section(*section);
  }

  _result.bin = _generator->generate_binary();
  return _result;
}

void assembler_c::add_error(const std::string &str)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  std::string e = "Error (line: ";
  e += std::to_string(_current_line_number);
  e += ") : ";
  e += str;

  if (_result.errors == std::nullopt) {
    std::vector<std::string> errors;
    errors.push_back(e);
    _result.errors = {errors};
  }
  else {
    _result.errors.value().push_back(e);
  }

  LOG(FATAL) << TAG("assembler") << "line: " << _current_line_number << ": "
             << str << "\n";
}

void assembler_c::add_warning(const std::string &str)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  std::string e = "Warning (line: ";
  e += std::to_string(_current_line_number);
  e += ") : ";
  e += str;

  if (_result.warnings == std::nullopt) {
    std::vector<std::string> warnings;
    warnings.push_back(e);
    _result.warnings = {warnings};
    return;
  }
  _result.warnings.value().push_back(e);

  LOG(WARNING) << TAG("assembler") << "line: " << _current_line_number << ": "
               << str << "\n";
}

void assembler_c::add_debug(const std::string &str)
{
  LOG(DEBUG) << TAG("assembler") << "line: " << _current_line_number << ": "
             << str << "\n";
}

void assembler_c::add_trace(const std::string &str)
{
  LOG(TRACE) << TAG("assembler") << "line: " << _current_line_number << ": "
             << str << "\n";
}

void assembler_c::pre_scan()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  _expected_bin_size = 0;
  bool count_items = false;
  for (auto &line : _file_data) {
    _current_line_number++;
    auto chunks = chunk_line(line);
    if (chunks.empty()) {
      continue;
    }
    if (!count_items) {
      if (chunks[0] == ".code") {
        count_items = true;
        continue;
      }
      continue;
    }

    // Match a label
    //
    if (std::regex_match(chunks[0], std::regex("^[a-zA-Z0-9_]+:$"))) {
      if (chunks.size() != 1) {
        add_error("Malformed Label");
        continue;
      }

      std::string label_name =
          chunks[0].substr(0, chunks[0].find_first_of(':'));

      // Calculate label location within instructions by taking size of binary
      // and dividing it by the number of bytes per instruction
      _label_to_location[label_name] =
          _expected_bin_size / INSTRUCTION_SIZE_BYTES;

      add_debug(label_name);

      // Continue because labels aren't instructions
      continue;
    }

    // Add to length
    // Save the instruction bytes iterated to sanity check
    // at the end of the assembly run
    _expected_bin_size += INSTRUCTION_SIZE_BYTES;
  }

  _current_line_number = 0;
}

void assembler_c::parse(const std::vector<match_t> &function_match)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  if (_current_chunks.empty()) {
    return;
  }
  for (auto &match : function_match) {
    if (std::regex_match(_current_chunks[0], match.reg)) {
      if (!match.call()) {
        return;
      }
    }
  }
}

std::string assembler_c::remove_comments(const std::string &line)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  return line.substr(0, line.find_first_of(';'));
}

void assembler_c::assemble()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  std::ifstream ifs(_input_file);
  if (!ifs.is_open()) {
    add_error("Unable to open file for assembly");
    return;
  }

  std::string current;
  while (std::getline(ifs, current)) {
    _file_data.push_back(remove_comments(current));
  }
  ifs.close();

  pre_scan();

  for (auto &line : _file_data) {
    _current_line_number++;
    if (line.empty() || current.find_first_not_of(' ') != std::string::npos) {
      continue;
    }
    _current_processed_line_number++;
    _current_chunks.clear();
    _current_chunks = chunk_line(line);
    switch (_state) {
    case state_e::DIRECTIVE_DIGESTION:
      parse(_directive_match);
      break;
    case state_e::INSTRUCTION_DIGESTION:
      parse(_instruction_match);
      break;
    }
  }

  if (!_directive_checks.init) {
    add_error("Missing .init directive");
    return;
  }
  if (!_directive_checks.code) {
    add_error("Missing .code directive");
    return;
  }

  _result.stats.num_instructions = _ins_gen.get_number_instructions_generated();
}

assembled_t assemble(const std::string &input)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  assembler_c assembler(input);
  assembler.assemble();
  return assembler.get_result();
}

std::optional<uint64_t> assembler_c::get_label_address(const std::string label)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  if (_label_to_location.find(label) == _label_to_location.end()) {
    return std::nullopt;
  }
  return _label_to_location[label];
}

bool assembler_c::directive_init()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_processed_line_number != 1) {
    add_error(".init directive must be the first item in the asm file");
    return false;
  }

  if (_directive_checks.init) {
    add_error("Duplicate .init directive");
    return false;
  }

  if (_current_chunks.size() != 2) {
    add_error("Malformed .init");
    return false;
  }

  auto init_address = get_label_address(_current_chunks[1]);
  if (init_address == std::nullopt) {
    add_error("Label given to .init does not exist");
    return false;
  }

  _directive_checks.init = true;
  _generator = std::make_unique<libskiff::generator::binary_generator>();
  _generator->set_entry(*init_address);
  return true;
}

bool assembler_c::directive_code()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_directive_checks.code) {
    add_error("Duplicate .code directive");
    return false;
  }

  if (_current_chunks.size() != 1) {
    add_error("Malformed .code directive");
    return false;
  }

  _directive_checks.code = true;
  _state = state_e::INSTRUCTION_DIGESTION;
  return true;
}

bool assembler_c::directive_debug()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_directive_checks.debug) {
    add_error("Duplicate .debug directive");
    return false;
  }

  if (_current_chunks.size() != 2) {
    add_error("Malformed .debug directive");
    return false;
  }

  auto value = get_number<int>(_current_chunks[1]);
  if (value == std::nullopt) {
    add_error("Malformed value for .debug directive");
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
    add_error("Invalid debug level given to .debug directive : " +
              std::to_string(*value));
    return false;
  }

  add_debug("Set .debug level to " + std::to_string(*value));

  _generator->set_debug(converted);
  return true;
}

bool assembler_c::add_constant(std::string name,
                               libskiff::types::constant_type_e type,
                               std::vector<uint8_t> data)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  if (_constant_name_to_value.find(name) != _constant_name_to_value.end()) {
    add_error("Duplicate constant name '" + _current_chunks[1] + "'");
    return false;
  }

  uint64_t address = _generator->add_constant(type, data);
  _constant_name_to_value[name] =
      constant_value_t{.type = type, .address = address, .data = data};
  return true;
}

bool assembler_c::directive_string()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .string directive");
    return false;
  }

  auto value = _ins_gen.gen_string_constant(_current_chunks[2]);
  if (value == std::nullopt) {
    add_error("Unable to encode .string directive with value : " +
              _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1],
                      libskiff::types::constant_type_e::STRING, *value);
}

bool assembler_c::directive_float()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .float directive");
    return false;
  }

  auto fp_value = get_number<double>(_current_chunks[2]);

  if (fp_value == std::nullopt) {
    add_error("Unable to convert constant to double : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1],
                      libskiff::types::constant_type_e::FLOAT,
                      _ins_gen.generate_fp_constant(*fp_value));
  return false;
}

// These integer constants below could be made into a templated function thats
// fancy but this will work for now

bool assembler_c::directive_int_8()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .i8 directive");
    return false;
  }

  auto value = get_number<int64_t>(_current_chunks[2]);
  if (value > std::numeric_limits<int8_t>::max() ||
      value < std::numeric_limits<int8_t>::min()) {
    add_error("Out of range value given for i8 : " + _current_chunks[2]);
    return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], libskiff::types::constant_type_e::I8,
                      _ins_gen.generate_i8_constant(*value));
  return false;
}

bool assembler_c::directive_int_16()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .i16 directive");
    return false;
  }

  auto value = get_number<int64_t>(_current_chunks[2]);
  if (value > std::numeric_limits<int16_t>::max() ||
      value < std::numeric_limits<int16_t>::min()) {
    add_error("Out of range value given for i16 : " + _current_chunks[2]);
    return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], libskiff::types::constant_type_e::I16,
                      _ins_gen.generate_i16_constant(*value));
  return false;
}

bool assembler_c::directive_int_32()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .i32 directive");
    return false;
  }

  auto value = get_number<int64_t>(_current_chunks[2]);
  if (value > std::numeric_limits<int32_t>::max() ||
      value < std::numeric_limits<int32_t>::min()) {
    add_error("Out of range value given for i32 : " + _current_chunks[2]);
    return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], libskiff::types::constant_type_e::I32,
                      _ins_gen.generate_i32_constant(*value));
  return false;
}

bool assembler_c::directive_int_64()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .i64 directive");
    return false;
  }

  auto value = get_number<int64_t>(_current_chunks[2]);
  if (value > std::numeric_limits<int64_t>::max() ||
      value < std::numeric_limits<int64_t>::min()) {
    add_error("Out of range value given for i64 : " + _current_chunks[2]);
    return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], libskiff::types::constant_type_e::I64,
                      _ins_gen.generate_i64_constant(*value));
  return false;
}

bool assembler_c::directive_uint_8()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .u8 directive");
    return false;
  }

  auto value = get_number<uint64_t>(_current_chunks[2]);
  if (value > std::numeric_limits<uint8_t>::max() ||
      value < std::numeric_limits<uint8_t>::min()) {
    add_error("Out of range value given for u8 : " + _current_chunks[2]);
    return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to unsigned integer : " +
              _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], libskiff::types::constant_type_e::U8,
                      _ins_gen.generate_u8_constant(*value));
  return false;
}

bool assembler_c::directive_uint_16()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .u16 directive");
    return false;
  }

  auto value = get_number<uint64_t>(_current_chunks[2]);
  if (value > std::numeric_limits<uint16_t>::max() ||
      value < std::numeric_limits<uint16_t>::min()) {
    add_error("Out of range value given for u16 : " + _current_chunks[2]);
    return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to unsigned integer : " +
              _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], libskiff::types::constant_type_e::U16,
                      _ins_gen.generate_u16_constant(*value));
  return false;
}

bool assembler_c::directive_uint_32()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .u32 directive");
    return false;
  }

  auto value = get_number<uint64_t>(_current_chunks[2]);
  if (value > std::numeric_limits<uint32_t>::max() ||
      value < std::numeric_limits<uint32_t>::min()) {
    add_error("Out of range value given for u32 : " + _current_chunks[2]);
    return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to unsigned integer : " +
              _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], libskiff::types::constant_type_e::U32,
                      _ins_gen.generate_u32_constant(*value));
  return false;
}

bool assembler_c::directive_uint_64()
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .u64 directive");
    return false;
  }

  auto value = get_number<uint64_t>(_current_chunks[2]);
  if (value > std::numeric_limits<uint64_t>::max() ||
      value < std::numeric_limits<uint64_t>::min()) {
    add_error("Out of range value given for u64 : " + _current_chunks[2]);
    return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to unsigned integer : " +
              _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], libskiff::types::constant_type_e::U64,
                      _ins_gen.generate_u64_constant(*value));
  return false;
}

void assembler_c::add_instruction_bytes(std::vector<uint8_t> bytes)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  _generator->add_instruction(bytes);
}

std::optional<uint32_t> assembler_c::get_value(const std::string item)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";
  if (item.size() < 2) {
    return std::nullopt;
  }

  // Check if its a raw value
  if (item.starts_with('@')) {
    std::string s = item.substr(1, item.length());
    if (s.starts_with('-')) {
      auto v = get_number<int32_t>(s);
      if (v != std::nullopt) {
        return static_cast<uint32_t>(*v);
      }
      return std::nullopt;
    }
    else {
      return get_number<uint32_t>(s);
    }
  }

  // Check if its an address
  if (item.starts_with('&')) {
    std::string s = item.substr(1, item.length());

    // Check labels
    auto label_address = get_label_address(s);
    if (label_address != std::nullopt) {
      if (label_address > std::numeric_limits<uint32_t>::max()) {
        add_error("Requested mov of label address whose value exceeds that "
                  "able to be stored in a uint32_t");
        return std::nullopt;
      }
      return static_cast<uint32_t>(*label_address);
    }

    // Check constants
    if (_constant_name_to_value.find(s) != _constant_name_to_value.end()) {
      auto constant = _constant_name_to_value[s];
      if (constant.address > std::numeric_limits<uint32_t>::max()) {
        add_error("Requested mov of constant address whose value exceeds that "
                  "able to be stored in a uint32_t");
        return std::nullopt;
      }
      return static_cast<uint32_t>(constant.address);
    }
    return std::nullopt;
  }

  // Check if its a length
  if (item.starts_with('#')) {
    std::string s = item.substr(1, item.length());

    // Check labels
    auto label_address = get_label_address(s);
    if (label_address != std::nullopt) {
      return 8; // An address is 8 bytes, and all labels are an address
    }

    // Check constants
    if (_constant_name_to_value.find(s) != _constant_name_to_value.end()) {
      auto constant = _constant_name_to_value[s];
      auto length = static_cast<uint32_t>(constant.data.size());
      if (length > std::numeric_limits<uint32_t>::max()) {
        add_error("Requested mov of constant whose length exceeds that "
                  "able to be stored in a uint32_t");
        return std::nullopt;
      }
      return length;
    }
  }

  return std::nullopt;
}

bool assembler_c::build_nop()
{
  add_trace(__func__);
  add_instruction_bytes(_ins_gen.gen_nop());
  return true;
}

bool assembler_c::build_exit()
{
  add_trace(__func__);
  add_instruction_bytes(_ins_gen.gen_exit());
  return true;
}

bool assembler_c::build_blt()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed blt instruction");
    return false;
  }

  auto address = get_label_address(_current_chunks[3]);
  if (address == std::nullopt) {
    add_error("Unknown label given to blt instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[1]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_blt(*lhs, *rhs, *address));
  return true;
}

bool assembler_c::build_bgt()
{
  add_trace(__func__);

  if (_current_chunks.size() != 4) {
    add_error("Malformed bgt instruction");
    return false;
  }

  auto address = get_label_address(_current_chunks[3]);
  if (address == std::nullopt) {
    add_error("Unknown label given to bgt instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[1]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_bgt(*lhs, *rhs, *address));
  return true;
}

bool assembler_c::build_beq()
{
  add_trace(__func__);

  if (_current_chunks.size() != 4) {
    add_error("Malformed beq instruction");
    return false;
  }

  auto address = get_label_address(_current_chunks[3]);
  if (address == std::nullopt) {
    add_error("Unknown label given to beq instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[1]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_beq(*lhs, *rhs, *address));
  return true;
}

bool assembler_c::build_jmp()
{
  add_trace(__func__);
  if (_current_chunks.size() != 2) {
    add_error("Malformed jmp instruction");
    return false;
  }

  auto address = get_label_address(_current_chunks[1]);
  if (address == std::nullopt) {
    add_error("Unknown label given to jmp instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_jmp(*address));
  return true;
}

bool assembler_c::build_call()
{
  add_trace(__func__);
  if (_current_chunks.size() != 2) {
    add_error("Malformed call instruction");
    return false;
  }

  auto address = get_label_address(_current_chunks[1]);
  if (address == std::nullopt) {
    add_error("Unknown label given to call instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_call(*address));
  return true;
}

bool assembler_c::build_ret()
{
  add_instruction_bytes(_ins_gen.gen_ret());
  return true;
}

bool assembler_c::build_mov()
{
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed mov instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[1]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto value = get_value(_current_chunks[2]);
  if (value == std::nullopt) {
    add_error("Unable to retrieve value from source in mov instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_mov(*lhs, *value));
  return true;
}

bool assembler_c::build_add()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed add instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_add(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_sub()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed sub instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_sub(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_div()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed div instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_div(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_mul()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed mul instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_mul(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_addf()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed addf instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_addf(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_subf()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed subf instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_subf(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_divf()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed divf instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_divf(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_mulf()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed mulf instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_mulf(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_lsh()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed lsh instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_lsh(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_rsh()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed rsh instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_rsh(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_and()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed and instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_and(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_or()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed or instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_or(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_xor()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed xor instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[3]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_xor(*dest, *lhs, *rhs));
  return true;
}

bool assembler_c::build_not()
{
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed not instruction");
    return false;
  }

  auto dest = _ins_gen.get_register_value(_current_chunks[1]);
  if (dest == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto source = _ins_gen.get_register_value(_current_chunks[2]);
  if (source == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_not(*dest, *source));
  return true;
}

bool assembler_c::build_bltf()
{
  add_trace(__func__);
  if (_current_chunks.size() != 4) {
    add_error("Malformed bltf instruction");
    return false;
  }

  auto address = get_label_address(_current_chunks[3]);
  if (address == std::nullopt) {
    add_error("Unknown label given to bltf instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[1]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_bltf(*lhs, *rhs, *address));
  return true;
}

bool assembler_c::build_bgtf()
{
  add_trace(__func__);

  if (_current_chunks.size() != 4) {
    add_error("Malformed bgtf instruction");
    return false;
  }

  auto address = get_label_address(_current_chunks[3]);
  if (address == std::nullopt) {
    add_error("Unknown label given to bgtf instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[1]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_bgtf(*lhs, *rhs, *address));
  return true;
}

bool assembler_c::build_beqf()
{
  add_trace(__func__);

  if (_current_chunks.size() != 4) {
    add_error("Malformed beqf instruction");
    return false;
  }

  auto address = get_label_address(_current_chunks[3]);
  if (address == std::nullopt) {
    add_error("Unknown label given to beqf instruction");
    return false;
  }

  auto lhs = _ins_gen.get_register_value(_current_chunks[1]);
  if (lhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto rhs = _ins_gen.get_register_value(_current_chunks[2]);
  if (rhs == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_beqf(*lhs, *rhs, *address));
  return true;
}

bool assembler_c::build_asne()
{
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed asne instruction");
    return false;
  }

  auto expected = _ins_gen.get_register_value(_current_chunks[1]);
  if (expected == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto actual = _ins_gen.get_register_value(_current_chunks[2]);
  if (actual == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_asne(*expected, *actual));
  return true;
}

bool assembler_c::build_aseq()
{
  add_trace(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed asne instruction");
    return false;
  }

  auto expected = _ins_gen.get_register_value(_current_chunks[1]);
  if (expected == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  auto actual = _ins_gen.get_register_value(_current_chunks[2]);
  if (actual == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_aseq(*expected, *actual));
  return true;
}

bool assembler_c::build_push_w()
{
  add_trace(__func__);
  if (_current_chunks.size() != 2) {
    add_error("Malformed push_w instruction");
    return false;
  }

  auto reg = _ins_gen.get_register_value(_current_chunks[1]);
  if (reg == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_push_w(*reg));
  return true;
}

bool assembler_c::build_push_dw()
{
  add_trace(__func__);
  if (_current_chunks.size() != 2) {
    add_error("Malformed push_dw instruction");
    return false;
  }

  auto reg = _ins_gen.get_register_value(_current_chunks[1]);
  if (reg == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_push_dw(*reg));
  return true;
}

bool assembler_c::build_push_qw()
{
  add_trace(__func__);
  if (_current_chunks.size() != 2) {
    add_error("Malformed push_qw instruction");
    return false;
  }

  auto reg = _ins_gen.get_register_value(_current_chunks[1]);
  if (reg == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_push_qw(*reg));
  return true;
}

bool assembler_c::build_pop_w()
{
  add_trace(__func__);
  if (_current_chunks.size() != 2) {
    add_error("Malformed pop_w instruction");
    return false;
  }

  auto reg = _ins_gen.get_register_value(_current_chunks[1]);
  if (reg == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_pop_w(*reg));
  return true;
}

bool assembler_c::build_pop_dw()
{
  add_trace(__func__);
  if (_current_chunks.size() != 2) {
    add_error("Malformed pop_dw instruction");
    return false;
  }

  auto reg = _ins_gen.get_register_value(_current_chunks[1]);
  if (reg == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_pop_dw(*reg));
  return true;
}

bool assembler_c::build_pop_qw()
{
  add_trace(__func__);
  if (_current_chunks.size() != 2) {
    add_error("Malformed pop_qw instruction");
    return false;
  }

  auto reg = _ins_gen.get_register_value(_current_chunks[1]);
  if (reg == std::nullopt) {
    add_error("Invalid register given to instruction");
    return false;
  }

  add_instruction_bytes(_ins_gen.gen_pop_qw(*reg));
  return true;
}






















} // namespace assembler
} // namespace libskiff

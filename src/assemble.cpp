#include "assemble.hpp"

#include <fstream>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>
#include <unordered_map>

#include <libskiff/instruction_generator.hpp>

namespace skiff_assemble {

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
  assembled_t get_result() const { return _result; }
  void assemble();

private:
  static constexpr uint8_t INSTRUCTION_SIZE_BYTES = 8;

  enum class state_e {
    DIRECTIVE_DIGESTION,
    INSTRUCTION_DIGESTION,
  };

  enum class constant_type_e {
    STRING,
    FLOAT,
    U8,
    U16,
    U32,
    U64,
    I8,
    I16,
    I32,
    I64,
  };

  struct constant_value_t {
    constant_type_e type;
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
    bool data;
    bool debug;
    bool data_first;
  };

  struct directive_options_t {
    std::string init_label;
    uint8_t debug_level;
  };

  const std::string &_input_file;
  uint64_t _current_line_number{0};
  uint64_t _constant_address_counter{0};
  state_e _state{state_e::DIRECTIVE_DIGESTION};
  assembled_t _result{.stats = {.num_instructions = 0},
                      .errors = std::nullopt,
                      .warnings = std::nullopt,
                      .bin = std::nullopt};
  std::vector<std::string> _file_data;
  std::vector<match_t> _directive_match;
  std::vector<match_t> _instruction_match;

  directive_checks_t _directive_checks = {false, false, false, false, false};
  std::unordered_map<std::string, uint64_t> _label_to_location;
  std::unordered_map<std::string, constant_value_t> _constant_name_to_value;
  uint64_t _expected_bin_size{0};
  std::vector<std::string> _current_chunks;
  directive_options_t _directive_options;
  libskiff::instructions::instruction_generator_c _ins_gen;

  std::string remove_comments(const std::string &str);
  void add_error(const std::string &str);
  void add_warning(const std::string &str);
  void add_debug(const std::string &str);
  void pre_scan();
  void parse(const std::vector<match_t> &function_match);

  bool add_constant(std::string name, constant_type_e type,
                    std::vector<uint8_t> value);
  bool directive_init();
  bool directive_data();
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

  bool build_nop();
  bool build_blt();
  bool build_bgt();
  bool build_beq();
  bool build_jmp();
  bool build_call();
  bool build_ret();
};

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
} // namespace

assembler_c::assembler_c(const std::string &input) : _input_file(input)
{
  _directive_options = {"", 0};
  _directive_match = {
      match_t{std::regex("^\\.init$"),
              std::bind(&skiff_assemble::assembler_c::directive_init, this)},
      match_t{std::regex("^\\.data$"),
              std::bind(&skiff_assemble::assembler_c::directive_data, this)},
      match_t{std::regex("^\\.debug$"),
              std::bind(&skiff_assemble::assembler_c::directive_debug, this)},
      match_t{std::regex("^\\.code$"),
              std::bind(&skiff_assemble::assembler_c::directive_code, this)},
      match_t{std::regex("^\\.string$"),
              std::bind(&skiff_assemble::assembler_c::directive_string, this)},
      match_t{std::regex("^\\.float$"),
              std::bind(&skiff_assemble::assembler_c::directive_float, this)},
      match_t{std::regex("^\\.i8$"),
              std::bind(&skiff_assemble::assembler_c::directive_int_8, this)},
      match_t{std::regex("^\\.i16$"),
              std::bind(&skiff_assemble::assembler_c::directive_int_16, this)},
      match_t{std::regex("^\\.i32$"),
              std::bind(&skiff_assemble::assembler_c::directive_int_32, this)},
      match_t{std::regex("^\\.i64$"),
              std::bind(&skiff_assemble::assembler_c::directive_int_64, this)},
      match_t{std::regex("^\\.u8$"),
              std::bind(&skiff_assemble::assembler_c::directive_uint_8, this)},
      match_t{std::regex("^\\.u16$"),
              std::bind(&skiff_assemble::assembler_c::directive_uint_16, this)},
      match_t{std::regex("^\\.u32$"),
              std::bind(&skiff_assemble::assembler_c::directive_uint_32, this)},
      match_t{std::regex("^\\.u64$"),
              std::bind(&skiff_assemble::assembler_c::directive_uint_64, this)},
  };
  _instruction_match = {
      match_t{std::regex("^nop$"),
              std::bind(&skiff_assemble::assembler_c::build_nop, this)},
      match_t{std::regex("^blt"),
              std::bind(&skiff_assemble::assembler_c::build_blt, this)},
      match_t{std::regex("^bgt"),
              std::bind(&skiff_assemble::assembler_c::build_bgt, this)},
      match_t{std::regex("^beq"),
              std::bind(&skiff_assemble::assembler_c::build_beq, this)},
      match_t{std::regex("^jmp"),
              std::bind(&skiff_assemble::assembler_c::build_jmp, this)},
      match_t{std::regex("^call"),
              std::bind(&skiff_assemble::assembler_c::build_call, this)},
      match_t{std::regex("^ret"),
              std::bind(&skiff_assemble::assembler_c::build_ret, this)},
  };
}

void assembler_c::add_error(const std::string &str)
{
  std::string e = "Error (line: ";
  e += std::to_string(_current_line_number);
  e += ") : ";
  e += str;

  if (_result.errors == std::nullopt) {
    std::vector<std::string> errors;
    errors.push_back(e);
    _result.errors = {errors};
    return;
  }
  _result.errors.value().push_back(e);
}

void assembler_c::add_warning(const std::string &str)
{
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
}

void assembler_c::add_debug(const std::string &str)
{
  std::cout << "DEBUG (line: " << _current_line_number << ") : " << str
            << std::endl;
}

void assembler_c::pre_scan()
{
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
      _label_to_location[label_name] = _expected_bin_size;

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
  return line.substr(0, line.find_first_of(';'));
}

void assembler_c::assemble()
{
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
  }
  if (!_directive_checks.code) {
    add_error("Missing .code directive");
  }
  if (!_directive_checks.data) {
    add_error("Missing .data directive");
  }
  if (!_directive_checks.init) {
    add_error("Misplaced .data directive - it must exist before all else");
  }

  // ----------------------------------------------------
  //    Remove all this
  // ----------------------------------------------------

  for (auto item : _constant_name_to_value) {

    std::cout << "Constant : " << item.first
              << ", type = " << (int)item.second.type
              << ", address = " << item.second.address << std::endl;
  }

  // ----------------------------------------------------
  _result.stats.num_instructions = _ins_gen.get_number_instructions_generated();
}

assembled_t assemble(const std::string &input)
{
  assembler_c assembler(input);
  assembler.assemble();
  return assembler.get_result();
}

bool assembler_c::directive_init()
{
  add_debug(__func__);
  if (_directive_checks.init) {
    add_error("Duplicate .init directive");
    return false;
  }

  if (_current_chunks.size() != 2) {
    add_error("Malformed .init");
    return false;
  }

  if (_label_to_location.find(_current_chunks[1]) == _label_to_location.end()) {
    add_error("Label given to .init does not exist");
    return false;
  }

  _directive_options.init_label = _current_chunks[1];
  _directive_checks.init = true;
  return true;
}

bool assembler_c::directive_code()
{
  add_debug(__func__);
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

bool assembler_c::directive_data()
{
  add_debug(__func__);
  if (_directive_checks.data) {
    add_error("Duplicate .data directive");
    return false;
  }
  if (_current_line_number != 1) {
    add_error(".data directive must be the first item in the asm file");
    return false;
  }
  else {
    _directive_checks.data_first = true;
  }
  _directive_checks.data = true;
  return true;
}

bool assembler_c::directive_debug()
{
  add_debug(__func__);
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

  switch (*value) {
  case 0:
  case 1:
  case 2:
  case 3:
    break;
  default:
    add_error("Invalid debug level given to .debug directive : " +
              std::to_string(*value));
    return false;
  }

  _directive_checks.debug = true;
  _directive_options.debug_level = *value;
  add_debug("Set .debug level to " +
            std::to_string(_directive_options.debug_level));
  return true;
}

bool assembler_c::add_constant(std::string name, constant_type_e type,
                               std::vector<uint8_t> data)
{
  if (_constant_name_to_value.find(name) != _constant_name_to_value.end()) {
    add_error("Duplicate constant name '" + _current_chunks[1] + "'");
    return false;
  }

  _constant_name_to_value[name] = constant_value_t{
      .type = type, .address = _constant_address_counter, .data = data};

  _constant_address_counter += data.size();
  return true;
}

bool assembler_c::directive_string()
{
  add_debug(__func__);
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

  return add_constant(_current_chunks[1], constant_type_e::STRING, *value);
}

bool assembler_c::directive_float()
{
  add_debug(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .float directive");
    return false;
  }

  auto fp_value = get_number<double>(_current_chunks[2]);

  if (fp_value == std::nullopt) {
    add_error("Unable to convert constant to double : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], constant_type_e::FLOAT,
                      _ins_gen.generate_fp_constant(*fp_value));
  return false;
}

// These integer constants below could be made into a templated function thats fancy
// but this will work for now

bool assembler_c::directive_int_8()
{
  add_debug(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .i8 directive");
    return false;
  }

  auto value = get_number<int64_t>(_current_chunks[2]);
  if(value > std::numeric_limits<int8_t>::max() || 
     value < std::numeric_limits<int8_t>::min()) {
       add_error("Out of range value given for i8 : " + _current_chunks[2] );
       return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], constant_type_e::I8,
                      _ins_gen.generate_i8_constant(*value));
  return false;
}

bool assembler_c::directive_int_16()
{
  add_debug(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .i16 directive");
    return false;
  }

  auto value = get_number<int64_t>(_current_chunks[2]);
  if(value > std::numeric_limits<int16_t>::max() || 
     value < std::numeric_limits<int16_t>::min()) {
       add_error("Out of range value given for i16 : " + _current_chunks[2] );
       return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], constant_type_e::I16,
                      _ins_gen.generate_i16_constant(*value));
  return false;
}

bool assembler_c::directive_int_32()
{
  add_debug(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .i32 directive");
    return false;
  }

  auto value = get_number<int64_t>(_current_chunks[2]);
  if(value > std::numeric_limits<int32_t>::max() || 
     value < std::numeric_limits<int32_t>::min()) {
       add_error("Out of range value given for i32 : " + _current_chunks[2] );
       return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], constant_type_e::I32,
                      _ins_gen.generate_i32_constant(*value));
  return false;
}

bool assembler_c::directive_int_64()
{
  add_debug(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .i64 directive");
    return false;
  }

  auto value = get_number<int64_t>(_current_chunks[2]);
  if(value > std::numeric_limits<int64_t>::max() || 
     value < std::numeric_limits<int64_t>::min()) {
       add_error("Out of range value given for i64 : " + _current_chunks[2] );
       return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], constant_type_e::I64,
                      _ins_gen.generate_i64_constant(*value));
  return false;
}

bool assembler_c::directive_uint_8()
{
  add_debug(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .u8 directive");
    return false;
  }

  auto value = get_number<uint64_t>(_current_chunks[2]);
  if(value > std::numeric_limits<uint8_t>::max() || 
     value < std::numeric_limits<uint8_t>::min()) {
       add_error("Out of range value given for u8 : " + _current_chunks[2] );
       return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to unsigned integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], constant_type_e::U8,
                      _ins_gen.generate_u8_constant(*value));
  return false;
}

bool assembler_c::directive_uint_16()
{
  add_debug(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .u16 directive");
    return false;
  }

  auto value = get_number<uint64_t>(_current_chunks[2]);
  if(value > std::numeric_limits<uint16_t>::max() || 
     value < std::numeric_limits<uint16_t>::min()) {
       add_error("Out of range value given for u16 : " + _current_chunks[2] );
       return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to unsigned integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], constant_type_e::U16,
                      _ins_gen.generate_u16_constant(*value));
  return false;
}

bool assembler_c::directive_uint_32()
{
  add_debug(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .u32 directive");
    return false;
  }

  auto value = get_number<uint64_t>(_current_chunks[2]);
  if(value > std::numeric_limits<uint32_t>::max() || 
     value < std::numeric_limits<uint32_t>::min()) {
       add_error("Out of range value given for u32 : " + _current_chunks[2] );
       return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to unsigned integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], constant_type_e::U32,
                      _ins_gen.generate_u32_constant(*value));
  return false;
}

bool assembler_c::directive_uint_64()
{
  add_debug(__func__);
  if (_current_chunks.size() != 3) {
    add_error("Malformed .u64 directive");
    return false;
  }

  auto value = get_number<uint64_t>(_current_chunks[2]);
  if(value > std::numeric_limits<uint64_t>::max() || 
     value < std::numeric_limits<uint64_t>::min()) {
       add_error("Out of range value given for u64 : " + _current_chunks[2] );
       return false;
  }

  if (value == std::nullopt) {
    add_error("Unable to convert constant to unsigned integer : " + _current_chunks[2]);
    return false;
  }

  return add_constant(_current_chunks[1], constant_type_e::U64,
                      _ins_gen.generate_u64_constant(*value));
  return false;
}

void assembler_c::add_instruction_bytes(std::vector<uint8_t> bytes)
{
  if(_result.bin == std::nullopt) {
    _result.bin = std::vector<uint8_t>();
  }
  _result.bin->insert(_result.bin->end(), bytes.begin(), bytes.end());
}

bool assembler_c::build_nop()
{
  add_debug(__func__);
  add_instruction_bytes(_ins_gen.gen_nop());
  return true;
}

bool assembler_c::build_blt()
{
  add_debug(__func__);
  return false;
}

bool assembler_c::build_bgt()
{
  add_debug(__func__);
  return false;
}

bool assembler_c::build_beq()
{
  add_debug(__func__);
  return false;
}

bool assembler_c::build_jmp()
{
  add_debug(__func__);
  return false;
}

bool assembler_c::build_call()
{
  add_debug(__func__);
  return false;
}

bool assembler_c::build_ret()
{
  add_debug(__func__);
  return false;
}

} // namespace skiff_assemble

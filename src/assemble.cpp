#include "assemble.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>

namespace skiff_assemble {

namespace {

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

  struct match_t {
    std::regex reg;
    std::function<bool(const std::vector<std::string> &line)> call;
  };

  struct directive_checks_t {
    bool init;
    bool code;
    bool data;
    bool data_first;
  };

  const std::string &_input_file;
  uint64_t _current_line_number{0};
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
  uint64_t _expected_bin_size{0};

  std::string remove_comments(const std::string &str);
  void add_error(const std::string &str);
  void add_warning(const std::string &str);
  void pre_scan();
  void parse(const std::vector<std::string> chunks,
             const std::vector<match_t> &function_match);

  bool directive_init(const std::vector<std::string> &line);
  bool directive_data(const std::vector<std::string> &line);
  bool directive_code(const std::vector<std::string> &line);

  bool build_nop(const std::vector<std::string> &line);
  bool build_blt(const std::vector<std::string> &line);
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
  _directive_match = {
      match_t{std::regex("^\\.init$"),
              std::bind(&skiff_assemble::assembler_c::directive_init, this,
                        std::placeholders::_1)},
      match_t{std::regex("^\\.data$"),
              std::bind(&skiff_assemble::assembler_c::directive_data, this,
                        std::placeholders::_1)},
      match_t{std::regex("^\\.code$"),
              std::bind(&skiff_assemble::assembler_c::directive_code, this,
                        std::placeholders::_1)},
  };
  _instruction_match = {
      match_t{std::regex("^nop$"),
              std::bind(&skiff_assemble::assembler_c::build_nop, this,
                        std::placeholders::_1)},
      match_t{std::regex("^blt"),
              std::bind(&skiff_assemble::assembler_c::build_blt, this,
                        std::placeholders::_1)},
  };
}

void assembler_c::add_error(const std::string &str)
{
  std::string e = "Error (Line ";
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
  std::string e = "Warning (Line ";
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

void assembler_c::pre_scan()
{
  _expected_bin_size = 0;
  bool count_items = false;
  for(auto &line: _file_data) {
    auto chunks = chunk_line(line);
    if(chunks.empty()) {
      continue;
    }
    if(!count_items) {
      if(chunks[0] == ".code"){
        count_items = true;
        continue;
      }
      continue;
    }

    // Match a label
    //
    if(std::regex_match(chunks[0], std::regex("^[a-zA-Z0-9_]+:$"))) {
      if(chunks.size() != 1) {
        add_error("Malformed Label");
        continue;
      }

      std::string label_name = chunks[0].substr(0, chunks[0].find_first_of(';'));
      _label_to_location[label_name] = _expected_bin_size;

      // Continue because labels aren't instructions
      continue;
    }

    // Add to length
    // Save the instruction bytes iterated to sanity check
    // at the end of the assembly run
    _expected_bin_size += INSTRUCTION_SIZE_BYTES;
  }
}

void assembler_c::parse(const std::vector<std::string> chunks,
                        const std::vector<match_t> &function_match)
{
  if (chunks.empty()) {
    return;
  }
  for (auto &match : function_match) {
    if (std::regex_match(chunks[0], match.reg)) {
      if (!match.call(chunks)) {
        return;
      }
    }
  }
}

std::string assembler_c::remove_comments(const std::string& line) {
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
    current = remove_comments(current);
    if (!current.empty() &&
        current.find_first_not_of(' ') != std::string::npos) {
      _file_data.push_back(current);
    }
  }
  ifs.close();

  pre_scan();

  for (auto &line : _file_data) {
    _current_line_number++;
    switch (_state) {
    case state_e::DIRECTIVE_DIGESTION:
      parse(chunk_line(line), _directive_match);
      break;
    case state_e::INSTRUCTION_DIGESTION:
      parse(chunk_line(line), _instruction_match);
      break;
    }
  }




  for(auto &item : _label_to_location) {
    std::cout << "Label : " << item.first << ", Location : " << item.second << std::endl;
  }


}

assembled_t assemble(const std::string &input)
{
  assembler_c assembler(input);
  assembler.assemble();
  return assembler.get_result();
}

bool assembler_c::directive_init(const std::vector<std::string> &line)
{
  std::cout << "Directive: .init" << std::endl;

  if (_directive_checks.init) {

    // Duplicate .init directives
  }

  // Needs exactly 2 items in line. [.init label]

  _directive_checks.init = true;
  return false;
}

bool assembler_c::directive_code(const std::vector<std::string> &line)
{
  std::cout << "Directive: .code" << std::endl;

  if (_directive_checks.code) {
    add_error("Duplicate .code directive");
    return false;
  }

  if (line.size() != 1) {
    add_error("Malformed .code directive");
    return false;
  }

  _directive_checks.code = true;

  _state = state_e::INSTRUCTION_DIGESTION;
  return true;
}

bool assembler_c::directive_data(const std::vector<std::string> &line)
{
  std::cout << "Directive: .data" << std::endl;

  if (_directive_checks.data) {
    // Duplicate .data
  }

  if (_current_line_number != 1) {
    // .data must be the first thing in the file
  }

  _directive_checks.data = true;
  return false;
}

bool assembler_c::build_nop(const std::vector<std::string> &line)
{
  std::cout << "Instruction: nop" << std::endl;
  return false;
}

bool assembler_c::build_blt(const std::vector<std::string> &line)
{
  std::cout << "Instruction: blt" << std::endl;


  for(auto &el : line) {
    std::cout << el << std::endl;
  }

  return false;
}

} // namespace skiff_assemble

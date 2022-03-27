#include "assemble.hpp"

#include <iostream>
#include <regex>
#include <fstream>

namespace skiff_assemble {

namespace {

class assembler_c {
public:
  assembler_c(const std::string &input);
  assembled_t get_result() const { return _result; }
  void assemble();

private:

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

  directive_checks_t _directive_checks = { false, false, false, false };

  void add_error(const std::string &str);
  void add_warning(const std::string &str);
  void parse(const std::vector<std::string> chunks, const std::vector<match_t>& function_match);


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
  _directive_match = {match_t{std::regex("^\\.init$"),
                             std::bind(&skiff_assemble::assembler_c::directive_init,
                                       this, std::placeholders::_1)},
                     match_t{std::regex("^\\.data$"),
                             std::bind(&skiff_assemble::assembler_c::directive_data,
                                       this, std::placeholders::_1)},
                     match_t{std::regex("^\\.code$"),
                             std::bind(&skiff_assemble::assembler_c::directive_code,
                                       this, std::placeholders::_1)},
  };
  _instruction_match = {match_t{std::regex("^nop$"),
                             std::bind(&skiff_assemble::assembler_c::build_nop,
                                       this, std::placeholders::_1)},
                     match_t{std::regex("^blt"),
                             std::bind(&skiff_assemble::assembler_c::build_blt,
                                       this, std::placeholders::_1)},
  };
}

void assembler_c::add_error(const std::string& str)
{
  if(_result.errors == std::nullopt) {
    std::vector<std::string> errors;
    errors.push_back(str);
    _result.errors = {errors};
    return;
  }
  _result.errors.value().push_back(str);
}

void assembler_c::add_warning(const std::string& str)
{
  if(_result.warnings == std::nullopt) {
    std::vector<std::string> warnings;
    warnings.push_back(str);
    _result.warnings = {warnings};
    return;
  }
  _result.warnings.value().push_back(str);
}

void assembler_c::parse(const std::vector<std::string> chunks, const std::vector<match_t>& function_match)
{
  if(chunks.empty()) {
    return;
  }
  for(auto& match: function_match) {
    if(std::regex_match(chunks[0], match.reg)) {
      if(!match.call(chunks)) {
        return;
      }
    }
  }
}

void assembler_c::assemble()
{
  std::ifstream ifs(_input_file);
  if(!ifs.is_open()) {
    add_error("Unable to open file for assembly");
    return;
  }

  std::string current;
  while (std::getline(ifs, current))
  {
      _file_data.push_back(current);
  }
  ifs.close();

  for(auto &line : _file_data) {
    _current_line_number++;
    switch(_state) {
    case state_e::DIRECTIVE_DIGESTION:
      parse(chunk_line(line), _directive_match);
      break;
    case state_e::INSTRUCTION_DIGESTION:
      parse(chunk_line(line), _instruction_match);
      break;
    }
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

  if(_directive_checks.init) {

    // Duplicate .init directives
  }

  // Needs exactly 2 items in line. [.init label]

  _directive_checks.init = true;
  return false;
}

bool assembler_c::directive_code(const std::vector<std::string> &line)
{
  std::cout << "Directive: .code" << std::endl;

  if( _state != state_e::DIRECTIVE_DIGESTION) {
    std::string error = std::to_string(_current_line_number) + ": Duplicate .code directives";
    add_error(error);
    return false;
  }

  if(line.size() != 1) {
    std::string error = std::to_string(_current_line_number) + ": Malformed .code directive";
    add_error(error);
    return false;
  }

  _directive_checks.code = true;

  _state = state_e::INSTRUCTION_DIGESTION;
  return true;
}

bool assembler_c::directive_data(const std::vector<std::string> &line)
{
  std::cout << "Directive: .data" << std::endl;

  if(_directive_checks.data) {
    // Duplicate .data
  }

  if(_current_line_number != 1) {
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
  return false;
}

} // namespace skiff_assemble

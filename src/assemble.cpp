#include "assemble.hpp"

#include <iostream>

namespace skiff_assemble {

namespace {

class assembler_c {
public:
  assembler_c(const std::string &input) : _input_file(input) {}
  assembled_t get_result() const { return _result; }
  void assemble();

private:
  const std::string &_input_file;
  assembled_t _result{.stats = {.num_instructions = 0},
                      .errors = std::nullopt,
                      .warnings = std::nullopt,
                      .bin = std::nullopt};
};

} // namespace

void assembler_c::assemble()
{
  std::cout << "Perform assembly here" << std::endl;
}

assembled_t assemble(const std::string &input)
{
  assembler_c assembler(input);
  assembler.assemble();
  return assembler.get_result();
}

} // namespace skiff_assemble

#include <filesystem>
#include <iostream>
#include <vector>

#include "assemble.hpp"
#include "options.hpp"

void handle_assebmled_t(skiff_assemble::assembled_t assembled,
                        std::optional<std::string> output)
{
  if (assembled.bin != std::nullopt) {
    std::cout << "Generated " << assembled.bin.value().size() << " bytes"
              << std::endl;
  }

  if (assembled.warnings != std::nullopt) {
    std::cout << assembled.warnings.value().size() << " warnings were generated"
              << std::endl;
  }

  if (assembled.warnings != std::nullopt) {
    std::cout << assembled.errors.value().size() << " errors were generated"
              << std::endl;
  }

  // TODO: Make this an argument option to turn on at one point

  std::cout << assembled.stats.num_instructions
            << " number of instructions were read in" << std::endl;

  if (output != std::nullopt) {
    std::cout << "Need to write this to file : " << output.value() << std::endl;
  }
}

int main(int argc, char **argv)
{
  auto opts =
      skiff_opt::build_options(std::vector<std::string>(argv + 1, argv + argc));

  // Display for now - remove later
  if (opts != std::nullopt) {
    skiff_opt::display_opts(opts.value());
  }

  if (opts->assemble_file != std::nullopt) {

    if (!std::filesystem::is_regular_file(
            opts->assemble_file.value().file_in)) {
      std::cout << "Error: Input file '" << opts->assemble_file.value().file_in
                << "' does not exist" << std::endl;
      std::exit(EXIT_FAILURE);
    }

    auto result = skiff_assemble::assemble(opts->assemble_file.value().file_in);

    handle_assebmled_t(result, opts->assemble_file->file_out);
  }

  std::cout << "No input given. Use -h for help" << std::endl;

  return 0;
}

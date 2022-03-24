#ifndef SKIFF_APP_OPTIONS_HPP
#define SKIFF_APP_OPTIONS_HPP

#include <optional>
#include <string>
#include <vector>

namespace skiff_opt {
struct assemble_t {
  std::string file_in;
  std::optional<std::string> file_out;
};

struct options_t {
  std::optional<assemble_t> assemble_file;
};

static void display_opts(skiff_opt::options_t opts)
{
  if (opts.assemble_file != std::nullopt) {
    std::cout << "Asked to assemble file : "
              << opts.assemble_file.value().file_in << std::endl;
    if (opts.assemble_file.value().file_out != std::nullopt) {
      std::cout << "Asked to output the file as : "
                << opts.assemble_file.value().file_out.value() << std::endl;
    }
  }
}

static std::optional<options_t> build_options(std::vector<std::string> opts)
{
  options_t options{std::nullopt};
  for (auto i = 0; i < opts.size(); i++) {

    // Assemble a file
    if (opts[i] == "-a" || opts[i] == "--assemble") {
      if (i + 1 > opts.size()) {
        std::cout << "Expected file name for 'assemble' instruction"
                  << std::endl;
        return std::nullopt;
      }
      if (options.assemble_file != std::nullopt) {
        std::cout
            << "Only one instance of an 'assemble' instruction can be given"
            << std::endl;
        return std::nullopt;
      }
      options.assemble_file = {.file_in = opts[i + 1],
                               .file_out = std::nullopt};
      i++; // Skip over the file name read in
      continue;
    }

    // Assemble file output name
    if (opts[i] == "-o" || opts[i] == "--out") {
      if (options.assemble_file == std::nullopt) {
        std::cout << "Expected '-a' or -'-assemble' prior to '-o' or '--out' "
                     "for output command"
                  << std::endl;
        return std::nullopt;
      }
      if (i + 1 > opts.size()) {
        std::cout << "Expected file name for 'out' instruction" << std::endl;
        return std::nullopt;
      }
      options.assemble_file->file_out = {opts[i + 1]};
      i++;
      continue;
    }
  }
  return {options};
}
} // namespace skiff_opt

#endif

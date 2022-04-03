#ifndef SKIFF_APP_OPTIONS_HPP
#define SKIFF_APP_OPTIONS_HPP

#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <libskiff/logging/aixlog.hpp>

namespace skiff_opt {
struct assemble_t {
  std::string file_in;
  std::optional<std::string> file_out;
  bool display_stats;
};

struct options_t {
  std::optional<assemble_t> assemble_file;
  AixLog::Severity log_level;
  std::vector<std::string> suspected_bin;
};

static void show_usage()
{
  std::cout << "[-a | --assemble ] <file>\t\tAssemble a file\n"
               "[-o | --out      ] <file>\t\tOutput file for assemble command\n"
               "[-s | --stats    ] \t\t\tDisplay statistics\n"
               "[-l | --loglevel ] \n\t[trace|debug|info|warn|error]\tDisplay statistics\n";
}

static std::optional<options_t> build_options(std::vector<std::string> opts)
{
  options_t options{std::nullopt, AixLog::Severity::fatal};
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
                               .file_out = std::nullopt,
                               .display_stats = false};
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

    // Log level
    if (opts[i] == "-l" || opts[i] == "--log") {
      if (i + 1 > opts.size()) {
        std::cout << "Expected loglevel for 'log' instruction" << std::endl;
        return std::nullopt;
      }
      
      if (opts[i + 1] == "trace") { options.log_level = AixLog::Severity::trace; }
      else if (opts[i + 1] == "debug") { options.log_level = AixLog::Severity::debug; }
      else if (opts[i + 1] == "info") { options.log_level = AixLog::Severity::info; }
      else if (opts[i + 1] == "warn") { options.log_level = AixLog::Severity::warning; }
      else if (opts[i + 1] == "error") { options.log_level = AixLog::Severity::error; }
      else {
        std::cout << "Invalid level '" << opts[i + 1] 
        << "' given to '" << opts[i] << "' instruction" << std::endl;
        std::exit(EXIT_FAILURE);
      }

      i++;
      continue;
    }

    // Toggle stats display
    if (opts[i] == "-s" || opts[i] == "--stats") {
      options.assemble_file->display_stats = true;
      continue;
    }

    // Help
    if (opts[i] == "-h" || opts[i] == "--help") {
      show_usage();
      std::exit(EXIT_SUCCESS);
    }

    // Anything else is suspected to be a bin
    //
    options.suspected_bin.push_back(opts[i]);
  }
  return {options};
}
} // namespace skiff_opt

#endif

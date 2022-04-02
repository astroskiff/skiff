#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "options.hpp"
#include <libskiff/types.hpp>
#include <libskiff/assembler/assemble.hpp>
#include <libskiff/bytecode/loader.hpp>
#include <libskiff/logging/aixlog.hpp>

void setup_logger(AixLog::Severity level)
{
  auto sink_cout = std::make_shared<AixLog::SinkCout>(level, "[#severity] (#tag) #message");
  auto sink_file = std::make_shared<AixLog::SinkFile>(level, "skiffd.log", "%Y-%m-%d %H-%M-%S.#ms | [#severity] (#tag) #message");
  AixLog::Log::init({sink_cout, sink_file});
}

void handle_assebmled_t(libskiff::assembler::assembled_t assembled,
                        std::optional<std::string> output, bool display_stats)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  if (assembled.warnings != std::nullopt) {
    std::cout << assembled.warnings.value().size() << " warnings were generated"
              << std::endl;
  }

  if (assembled.errors != std::nullopt) {
    std::cout << assembled.errors.value().size() << " errors were generated"
              << std::endl;
  }

  if (display_stats && assembled.bin != std::nullopt) {
    std::cout << assembled.stats.num_instructions
              << " number of instructions generated resulting in "
              << assembled.bin.value().size() << " bytes" << std::endl;
  }

  std::string out_name =
      (assembled.build_type == libskiff::types::binary_type_e::EXECUTABLE)
          ? "out.skiff_bin"
          : "out.skiff_lib";

  if (output != std::nullopt) {
    out_name = output.value();
  }

  std::ofstream fout(out_name, std::ios::out | std::ios::binary);
  fout.write(reinterpret_cast<const char *>(&assembled.bin.value()[0]),
             assembled.bin.value().size());

  LOG(DEBUG) << TAG("app") << "Binary written to file : " << out_name << "\n";
}

int main(int argc, char **argv)
{
  auto opts =
      skiff_opt::build_options(std::vector<std::string>(argv + 1, argv + argc));

  // Display for now - remove later
  if (opts != std::nullopt) {
    skiff_opt::display_opts(opts.value());
    setup_logger(opts->log_level);
  } else {
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal);
  }
  LOG(DEBUG) << TAG("app") << "Logger set up" << "\n";

  if (opts->assemble_file != std::nullopt) {
    
    // Ensure file input exists
    LOG(DEBUG) << TAG("app") << "Ensuring file exists\n";
    if (!std::filesystem::is_regular_file(
            opts->assemble_file.value().file_in)) {
      std::cout << "Error: Input file '" << opts->assemble_file.value().file_in
                << "' does not exist" << std::endl;
      std::exit(EXIT_FAILURE);
    }

    // Ensure any included libs exist
    if (!opts->assemble_file->libs.empty()) {
      LOG(DEBUG) << TAG("app") << "Ensuring given libraries exist\n";
      for (auto &item : opts->assemble_file.value().libs) {
        if (!std::filesystem::is_regular_file(item)) {
          std::cout << "Error: Assumed library file '" << item
                    << "' does not exist" << std::endl;
          std::exit(EXIT_FAILURE);
        }
      }
    }

    // Assemble input
    auto result = libskiff::assembler::assemble(
        opts->assemble_file.value().file_in, opts->assemble_file.value().libs);

    // Handle resulting object
    handle_assebmled_t(result, opts->assemble_file->file_out,
                       opts->assemble_file->display_stats);
    return 0;
  }

  std::cout << "No input given. Use -h for help" << std::endl;
  return 0;
}

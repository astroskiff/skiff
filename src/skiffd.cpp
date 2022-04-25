#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "options.hpp"
#include <libskiff/assembler/assemble.hpp>
#include <libskiff/bytecode/executable.hpp>
#include <libskiff/logging/aixlog.hpp>
#include <libskiff/machine/vm.hpp>
#include <libskiff/types.hpp>
#include <libskiff/defines.hpp>

void runtime_callback(libskiff::types::runtime_error_e error)
{
  std::string e;
  switch (error) {
  case libskiff::types::runtime_error_e::STACK_PUSH_ERROR:
    e = "Stack `push` error";
    break;
  case libskiff::types::runtime_error_e::STACK_POP_ERROR:
    e = "Stack `pop` error";
    break;
  case libskiff::types::runtime_error_e::RETURN_WITH_EMPTY_CALLSTACK:
    e = "Retrun executed with empty callstack";
    break;
  case libskiff::types::runtime_error_e::INSTRUCTION_PTR_OUT_OF_RANGE:
    e = "Instruction pointer moved out of range ";
    break;
  case libskiff::types::runtime_error_e::ILLEGAL_INSTRUCTION:
    e = "Illegal instruction";
    break;
  case libskiff::types::runtime_error_e::DIVIDE_BY_ZERO:
    e = "Divide by 0 detected";
    break;
  }
  LOG(FATAL) << TAG("runtime error") << e << "\n";
}

void setup_logger(AixLog::Severity level)
{
  auto sink_cout =
      std::make_shared<AixLog::SinkCout>(level, "[#severity] (#tag) #message");
  auto sink_file = std::make_shared<AixLog::SinkFile>(
      level, "skiffd.log",
      "%Y-%m-%d %H-%M-%S.#ms | [#severity] (#tag) #message");
  AixLog::Log::init({sink_cout, sink_file});
}

void handle_assebmled_t(libskiff::assembler::assembled_t assembled,
                        std::optional<std::string> output, bool display_stats)
{
  LOG(TRACE) << TAG("func") << __func__ << "\n";

  if (assembled.warnings != std::nullopt) {
    LOG(DEBUG) << TAG("app") << "Show warnings\n";
    std::cout << assembled.warnings.value().size() << " warnings were generated"
              << std::endl;
    for (auto &w : *assembled.warnings) {
      std::cout << w << std::endl;
    }
  }

  if (assembled.errors != std::nullopt) {
    LOG(DEBUG) << TAG("app") << "Show errors\n";
    std::cout << assembled.errors.value().size() << " errors were generated"
              << std::endl;
    for (auto &err : *assembled.errors) {
      std::cout << err << std::endl;
    }
    exit(1);
  }

  std::string out_name = "out.skiff";

  if (output != std::nullopt) {
    out_name = output.value();
  }


  if (display_stats && assembled.bin != std::nullopt) {
    LOG(DEBUG) << TAG("app") << "Show stats\n";
    std::cout << TERM_COLOR_CYAN << "---- Execution Statistics ----" << TERM_COLOR_END << std::endl;
    std::cout << TERM_COLOR_YELLOW << "Output file     : " << TERM_COLOR_END << out_name << std::endl;
    std::cout << TERM_COLOR_YELLOW << "Items assembled : " << TERM_COLOR_END << assembled.stats.num_instructions << std::endl;
    std::cout << TERM_COLOR_YELLOW << "Bytes produced  : " << TERM_COLOR_END << assembled.bin.value().size() << std::endl;
    std::cout << TERM_COLOR_CYAN << "------------------------------" << TERM_COLOR_END << std::endl;
  }


  if (assembled.bin == std::nullopt) {
    std::cout << "No resulting binary. Nothing to write" << std::endl;
    return;
  }

  LOG(DEBUG) << TAG("app") << "Write bin\n";
  std::ofstream fout(out_name, std::ios::out | std::ios::binary);
  fout.write(reinterpret_cast<const char *>(&assembled.bin.value()[0]),
             assembled.bin.value().size());

  LOG(DEBUG) << TAG("app") << "Binary written to file : " << out_name << "\n";
}

int run(const std::string &bin, bool show_statistics)
{
  std::optional<std::unique_ptr<libskiff::bytecode::executable_c>>
      loaded_binary = libskiff::bytecode::load_binary(bin);

  if (loaded_binary == std::nullopt) {
    LOG(FATAL) << TAG("app") << "Failed to load suspected binary file : " << bin
               << "\n";
    return 1;
  }

  libskiff::machine::vm_c vm;
  vm.set_runtime_callback(runtime_callback);

  if (!vm.load(std::move(loaded_binary.value()))) {
    LOG(FATAL) << TAG("app") << "Failed to load VM\n";
    return 1;
  }

  auto [value, code] = vm.execute();
  if (value != libskiff::machine::vm_c::execution_result_e::OKAY) {
    LOG(FATAL) << TAG("app") << "VM Died with an error\n";
    return code;
  }

  LOG(DEBUG) << TAG("app") << "VM returned code : " << code << "\n";

  if (show_statistics) {
    vm.display_runtime_statistics();
  }

  return code;
}

int main(int argc, char **argv)
{
  auto opts =
      skiff_opt::build_options(std::vector<std::string>(argv + 1, argv + argc));

  // Display for now - remove later
  if (opts != std::nullopt) {
    setup_logger(opts->log_level);
  }
  else {
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal);
  }
  LOG(DEBUG) << TAG("app") << "Logger set up"
             << "\n";

  if (opts->assemble_file != std::nullopt) {

    // Ensure file input exists
    LOG(DEBUG) << TAG("app") << "Ensuring file exists\n";
    if (!std::filesystem::is_regular_file(
            opts->assemble_file.value().file_in)) {
      std::cout << "Error: Input file '" << opts->assemble_file.value().file_in
                << "' does not exist" << std::endl;
      std::exit(EXIT_FAILURE);
    }

    // Assemble input
    auto result =
        libskiff::assembler::assemble(opts->assemble_file.value().file_in);

    // Handle resulting object
    handle_assebmled_t(result, opts->assemble_file->file_out,
                       opts->display_stats);
    return 0;
  }

  //  Check for bins
  if (!opts->suspected_bin.empty()) {
    for (auto &item : opts->suspected_bin) {
      if (auto i = run(item, opts->display_stats); i != 0) {
        return i;
      }
    }
    return 0;
  }

  std::cout << "No input given. Use -h for help" << std::endl;
  return 0;
}

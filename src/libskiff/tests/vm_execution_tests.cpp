/*
  This file contains a series of small programs that are written to file,
  assembled and then executed with the resulting code from the VM being checked
  to ensure that the instruction by itsself is working as intended.
*/

#include <libskiff/assembler/assemble.hpp>
#include <libskiff/bytecode/executable.hpp>
#include <libskiff/logging/aixlog.hpp>
#include <libskiff/machine/vm.hpp>

#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include <CppUTest/TestHarness.h>

TEST_GROUP(vm_execution_tests){};

TEST(vm_execution_tests, basic_tests)
{
  AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal);
  struct tc {
    std::string progam;
    int expected_return_code;
  };
  std::vector<tc> programs = {


      // Simple stack test
      {".init main\n"
       ".code\n"
       "main:\n"
       "mov i0 @20\n"
       "mov i1 @30\n"
       "mov i2 @50\n"
       "push_w i0\n"
       "push_dw i1\n"
       "push_qw i2\n"
       "pop_qw i3 \n"
       "aseq i2 i3 \n"
       "pop_dw i3 \n"
       "aseq i1 i3 \n"
       "pop_w i3 \n"
       "aseq i0 i3 \n"
       "mov i0 @100\n"
       "exit\n",
       100},

      // Load /stores
      {".init main\n"
       ".code\n"
       "main:\n"
       "mov i8 @33\n"
       "mov i9 @0\n"
       "mov i2 @1024\n"
       "alloc i3 i2\n"
       "aseq x1 op\n"
       "sw i3 i9 i8\n"
       "lw i3 i9 i6\n"
       "aseq i6 i8\n"
       "exit\n",
       0},
      {".init main\n"
       ".code\n"
       "main:\n"
       "mov i8 @33\n"
       "mov i9 @0\n"
       "mov i2 @1024\n"
       "alloc i3 i2\n"
       "aseq x1 op\n"
       "sdw i3 i9 i8\n"
       "ldw i3 i9 i6\n"
       "aseq i6 i8\n"
       "exit\n",
       0},
      {".init main\n"
       ".code\n"
       "main:\n"
       "mov i8 @33\n"
       "mov i9 @0\n"
       "mov i2 @1024\n"
       "alloc i3 i2\n"
       "aseq x1 op\n"
       "sqw i3 i9 i8\n"
       "lqw i3 i9 i6\n"
       "aseq i6 i8\n"
       "exit\n",
       0},

      // Load and check word aligned constants
      {".init main\n"
       ".i8  a 44\n"
       ".i16 b 99\n"
       ".code\n"
       "main:\n"
       "mov i9 @44\n"
       "mov i0 &a\n"
       "mov i1 @0\n"
       "lw i1 i0 i7\n"
       "aseq i9 i7\n"
       "mov i9 @99\n"
       "mov i9 @99\n"
       "mov i0 &b\n"
       "lw i1 i0 i7\n"
       "aseq i9 i7\n"
       "mov i0 @0\n"
       "exit\n",
       0}};

  for (auto &tc : programs) {
    {
      std::ofstream ofs("tmp.vm_execution_test.asm");
      if (!ofs.is_open()) {
        FAIL("Unable to open file for testing");
      }
      ofs << tc.progam;
    }
    {
      // Assemble the data
      auto result = libskiff::assembler::assemble("tmp.vm_execution_test.asm");

      if (result.warnings != std::nullopt) {
        CHECK_EQUAL(0, result.warnings.value().size());
      }
      if (result.errors != std::nullopt) {
        CHECK_EQUAL(0, result.errors.value().size());
      }
      if (result.bin == std::nullopt) {
        FAIL("No binary generated");
      }

      // Write out the bytes
      std::ofstream fout("tmp.vm_execution_test.bin",
                         std::ios::out | std::ios::binary);
      fout.write(reinterpret_cast<const char *>(&result.bin.value()[0]),
                 result.bin.value().size());
    }
    // Load the binary
    std::optional<std::unique_ptr<libskiff::binary::executable_c>>
        loaded_binary =
            libskiff::binary::load_binary("tmp.vm_execution_test.bin");

    CHECK_TRUE(loaded_binary != std::nullopt);

    libskiff::machine::vm_c skiff_vm;
    if (!skiff_vm.load(std::move(loaded_binary.value()))) {
      FAIL("Unable to load VM with binary");
    }

    auto [value, code] = skiff_vm.execute();
    if (value != libskiff::machine::vm_c::execution_result_e::OKAY) {
      std::string out = "VM Failed with code : " + std::to_string(code);
      FAIL(out.c_str());
    }

    CHECK_EQUAL_TEXT(tc.expected_return_code, code,
                     "VM did not return expected code");
  }
}

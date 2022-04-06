/*
  This file contains a series of small programs that are written to file,
  assembled and then executed with the resulting code from the VM being checked
  to ensure that the instruction by itsself is working as intended.
*/

#include <CppUTest/TestHarness.h>

#include <libskiff/assembler/assemble.hpp>
#include <libskiff/bytecode/executable.hpp>
#include <libskiff/logging/aixlog.hpp>
#include <libskiff/machine/vm.hpp>

#include <fstream>
#include <string>
#include <vector>

TEST_GROUP(vm_execution_tests){};

TEST(vm_execution_tests, basic_tests)
{
  AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal);
  struct tc {
    std::string progam;
    int expected_return_code;
  };
  std::vector<tc> programs = {

      // Integer arithmetic
      {".init main\n"
       ".code\n"
       "main:\n"
       "mov i1 @10\n"
       "mov i2 @2\n"
       "add i0 i1 i2\n"
       "exit\n",
       12},
      {".init main\n"
       ".code\n"
       "main:\n"
       "mov i3 @10\n"
       "mov i4 @2\n"
       "sub i0 i3 i4\n"
       "exit\n",
       8},
      {".init main\n"
       ".code\n"
       "main:\n"
       "mov i5 @10\n"
       "mov i6 @2\n"
       "mul i0 i5 i6\n"
       "exit\n",
       20},
      {".init main\n"
       ".code\n"
       "main:\n"
       "mov i7 @10\n"
       "mov i8 @2\n"
       "div i0 i7 i8\n"
       "exit\n",
       5},

      // Execute branches
      {".init main\n"
       ".code\n"
       "dest:\n"
       "exit\n"
       "main:\n"
       "mov i0 @10\n"
       "mov i1 @2\n"
       "bgt i0 i1 dest\n"
       "mov i0 @42\n"
       "exit\n",
       10},
      {".init main\n"
       ".code\n"
       "dest:\n"
       "exit\n"
       "main:\n"
       "mov i0 @10\n"
       "mov i1 @2\n"
       "blt i1 i0 dest\n"
       "mov i0 @42\n"
       "exit\n",
       10},
      {".init main\n"
       ".code\n"
       "dest:\n"
       "exit\n"
       "main:\n"
       "mov i0 @33\n"
       "mov i1 @33\n"
       "beq i0 i1 dest\n"
       "mov i0 @42\n"
       "exit\n",
       33},

      // Fall through branches
      {".init main\n"
       ".code\n"
       "dest:\n"
       "exit\n"
       "main:\n"
       "mov i0 @1\n"
       "mov i1 @10\n"
       "bgt i0 i1 dest\n"
       "mov i0 @42\n"
       "exit\n",
       42},
      {".init main\n"
       ".code\n"
       "dest:\n"
       "exit\n"
       "main:\n"
       "mov i0 @10\n"
       "mov i1 @2\n"
       "blt i0 i1 dest\n"
       "mov i0 @42\n"
       "exit\n",
       42},
      {".init main\n"
       ".code\n"
       "dest:\n"
       "exit\n"
       "main:\n"
       "mov i0 @33\n"
       "mov i1 @1\n"
       "beq i0 i1 dest\n"
       "mov i0 @99\n"
       "exit\n",
       99},

      // Jmp
      {".init main\n"
       ".code\n"
       "label:\n"
       "exit\n"
       "label_1:\n"
       "mov i0 @10\n"
       "main:\n"
       "mov i0 @33\n"
       "jmp label\n"
       "jmp label_1\n"
       "exit\n",
       33},

      // Call
      {".init main\n"
       ".code\n"
       "label:\n"
       "nop\n"
       "nop\n"
       "mov i0 @24\n"
       "ret\n"
       "label_1:\n"
       "mov i0 @10\n"
       "main:\n"
       "mov i0 @33\n"
       "call label\n"
       "exit\n",
       24},

      // Shifting
      {".init main\n"
       ".code\n"
       "main:\n"
       "mov i0 @2\n"
       "mov i1 @8\n"
       "lsh i0 i0 i1\n"
       "exit\n",
       512},
      {".init main\n"
       ".code\n"
       "main:\n"
       "mov i0 @512\n"
       "mov i1 @8\n"
       "rsh i0 i0 i1\n"
       "exit\n",
       2},

      /*
            TODO: add tests for [ and or xor not ]
      */

  };

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
#include <libskiff/assembler/assemble.hpp>
#include <libskiff/bytecode/executable.hpp>
#include <libskiff/logging/aixlog.hpp>
#include <libskiff/types.hpp>

#include <CppUTest/TestHarness.h>
#include <fstream>
#include <vector>

namespace {

struct tc_asm_t {
  std::string data;
  uint64_t num_items;
  libskiff::types::exec_debug_level_e debug_level;
};

} // namespace

TEST_GROUP(assembler_tests){void setup(){} void teardown(){}};

TEST(assembler_tests, all)
{
  AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal);

  std::vector<tc_asm_t> tcs;
  tcs.push_back(
      {".init main\n"
       ".debug 3\n"
       ".string moot \"This is a test of the emergency alert system\"\n"
       ".float pi 3.14159\n"
       ".float fart 33.0\n"
       ".i8  int_8   1\n"
       ".i16 int_16  2\n"
       ".i32 int_32  3\n"
       ".i64 int_64  4\n"
       ".u8  uint_8   11\n"
       ".u16 uint_16  12\n"
       ".u32 uint_32  13\n"
       ".u64 uint_64  14\n"
       ".code\n"
       "main:\n"
       "  nop\n"
       "  add i0 i0 i1\n"
       "  sub i0 i0 i1\n"
       "  div i0 i0 i1\n"
       "  mul i0 i0 i1\n"
       "  addf f0 f0 f1\n"
       "  subf f0 f0 f1\n"
       "  divf f0 f0 f1\n"
       "  mulf f0 f0 f1\n"
       "  lsh i1 i1 i2\n"
       "  rsh i1 i1 i2\n"
       "  or i1 i1 i2\n"
       "  xor i1 i1 i2\n"
       "  and i1 i1 i2\n"
       "  not i1 i1\n"
       "  exit\n",
       27, libskiff::types::exec_debug_level_e::EXTREME});

  tcs.push_back({".init main\n"
                 ".debug 1\n"
                 ".u64 uint_64  14\n"
                 ".code\n"
                 "main:\n"
                 "  exit\n",
                 2, libskiff::types::exec_debug_level_e::MINIMAL});

  tcs.push_back({".init main\n"
                 ".u64 uint_64  14\n"
                 ".code\n"
                 "main:\n"
                 "  exit\n",
                 2, libskiff::types::exec_debug_level_e::NONE});

  for (auto &tc : tcs) {

    // Write out the data
    {
      std::ofstream ofs("tmp.test.asm");
      if (!ofs.is_open()) {
        FAIL("Unable to open file for testing");
      }
      ofs << tc.data;
    }
    {
      // Assemble the data
      auto result = libskiff::assembler::assemble("tmp.test.asm");

      if (result.warnings != std::nullopt) {
        CHECK_EQUAL(0, result.warnings.value().size());
      }
      if (result.errors != std::nullopt) {
        CHECK_EQUAL(0, result.errors.value().size());
      }
      CHECK_EQUAL(tc.num_items, result.stats.num_instructions);

      // Write out the bytes
      std::ofstream fout("tmp.test.bin", std::ios::out | std::ios::binary);
      fout.write(reinterpret_cast<const char *>(&result.bin.value()[0]),
                 result.bin.value().size());
    }
    // Load the binary
    std::optional<std::unique_ptr<libskiff::bytecode::executable_c>>
        loaded_binary = libskiff::bytecode::load_binary("tmp.test.bin");

    CHECK_TRUE(loaded_binary != std::nullopt);
    CHECK_EQUAL(
        static_cast<int>(tc.debug_level),
        static_cast<int>(loaded_binary.value().get()->get_debug_level()));
  }
}

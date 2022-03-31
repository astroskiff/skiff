#include <CppUTest/TestHarness.h>

#include <libskiff/instruction_generator.hpp>
#include <libskiff/floating_point.hpp>
#include <libutil/generate_random.hpp>
#include <libutil/random_string.hpp>

#include <bitset>
#include <iostream>

namespace {
  struct tc_register_t {
    std::string reg;
    uint8_t value;
  };

  std::vector<tc_register_t> known_good_registers = {
    {"x0", 0x00},
    {"x1", 0x01},
    {"ip", 0x02},
    {"fp", 0x03},
    {"i0", 0x10},
    {"i1", 0x11},
    {"i2", 0x12},
    {"i3", 0x13},
    {"i4", 0x14},
    {"i5", 0x15},
    {"i6", 0x16},
    {"i7", 0x17},
    {"i8", 0x18},
    {"i9", 0x19},
    {"f0", 0x20},
    {"f1", 0x21},
    {"f2", 0x22},
    {"f3", 0x23},
    {"f4", 0x24},
    {"f5", 0x25},
    {"f6", 0x26},
    {"f7", 0x27},
    {"f8", 0x28},
    {"f9", 0x29},
  };

  std::vector<tc_register_t> bad_values = {
    {"x0", 0x40},
    {"x1", 0x41},
    {"ip", 0x42},
    {"fp", 0x43},
    {"i0", 0x40},
    {"i1", 0x41},
    {"i2", 0x42},
    {"i3", 0x43},
    {"i4", 0x44},
    {"i5", 0x45},
    {"i6", 0x46},
    {"i7", 0x47},
    {"i8", 0x48},
    {"i9", 0x40},
    {"f0", 0x40},
    {"f1", 0x41},
    {"f2", 0x42},
    {"f3", 0x43},
    {"f4", 0x44},
    {"f5", 0x45},
    {"f6", 0x46},
    {"f7", 0x47},
    {"f8", 0x48},
    {"f9", 0x40},
  };
}

TEST_GROUP(instruction_generator_tests)
{
  void setup()
  {
    // Init stuff
  }

  void teardown()
  {
    known_good_registers.clear();
    bad_values.clear();
  }
};

TEST(instruction_generator_tests, constants)
{
  constexpr uint8_t num_const_tests = 10;
  libskiff::instructions::instruction_generator_c gen;

  //  .u8
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    auto value = libutil::generate::generate_random_c<uint8_t>().get_range(
        std::numeric_limits<uint8_t>::min(),
        std::numeric_limits<uint8_t>::max());

    auto data = gen.generate_u8_constant(value);
    CHECK_TRUE(data.size() == 1);
    CHECK_EQUAL(value, data[0]);
  }

  //  .u16
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    auto value = libutil::generate::generate_random_c<uint16_t>().get_range(
        std::numeric_limits<uint16_t>::min(),
        std::numeric_limits<uint16_t>::max());

    auto data = gen.generate_u16_constant(value);
    CHECK_TRUE(data.size() == 2);

    uint16_t x = static_cast<uint16_t>(data[0]) << 8;
    x |= static_cast<uint16_t>(data[1]);
    CHECK_EQUAL(value, x);
  }

  //  .u32
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    auto value = libutil::generate::generate_random_c<uint32_t>().get_range(
        std::numeric_limits<uint32_t>::min(),
        std::numeric_limits<uint32_t>::max());

    auto data = gen.generate_u32_constant(value);
    CHECK_TRUE(data.size() == 4);

    uint32_t x = static_cast<uint32_t>(data[0]) << 24;
    x |= static_cast<uint32_t>(data[1]) << 16;
    x |= static_cast<uint32_t>(data[2]) << 8;
    x |= static_cast<uint32_t>(data[3]);
    CHECK_EQUAL(value, x);
  }

  //  .u64
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    auto value = libutil::generate::generate_random_c<uint64_t>().get_range(
        std::numeric_limits<uint64_t>::min(),
        std::numeric_limits<uint64_t>::max());

    auto data = gen.generate_u64_constant(value);
    CHECK_TRUE(data.size() == 8);

    uint64_t x = static_cast<uint64_t>(data[0]) << 56;
    x |= static_cast<uint64_t>(data[1]) << 48;
    x |= static_cast<uint64_t>(data[2]) << 40;
    x |= static_cast<uint64_t>(data[3]) << 32;
    x |= static_cast<uint64_t>(data[4]) << 24;
    x |= static_cast<uint64_t>(data[5]) << 16;
    x |= static_cast<uint64_t>(data[6]) << 8;
    x |= static_cast<uint64_t>(data[7]);
    CHECK_EQUAL(value, x);
  }

  //  .i8
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    auto value = libutil::generate::generate_random_c<int8_t>().get_range(
        std::numeric_limits<int8_t>::min(),
        std::numeric_limits<int8_t>::max());

    auto data = gen.generate_i8_constant(value);
    CHECK_TRUE(data.size() == 1);
    CHECK_EQUAL(value, static_cast<int8_t>(data[0]));
  }

  //  .i16
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    auto value = libutil::generate::generate_random_c<int16_t>().get_range(
        std::numeric_limits<int16_t>::min(),
        std::numeric_limits<int16_t>::max());

    auto data = gen.generate_i16_constant(value);
    CHECK_TRUE(data.size() == 2);

    int16_t x = static_cast<int16_t>(data[0]) << 8;
    x |= static_cast<int16_t>(data[1]);
    CHECK_EQUAL(value, x);
  }

  //  .i32
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    auto value = libutil::generate::generate_random_c<int32_t>().get_range(
        std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int32_t>::max());

    auto data = gen.generate_i32_constant(value);
    CHECK_TRUE(data.size() == 4);

    int32_t x = static_cast<int32_t>(data[0]) << 24;
    x |= static_cast<int32_t>(data[1]) << 16;
    x |= static_cast<int32_t>(data[2]) << 8;
    x |= static_cast<int32_t>(data[3]);
    CHECK_EQUAL(value, x);
  }

  //  .i64
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    auto value = libutil::generate::generate_random_c<int64_t>().get_range(
        std::numeric_limits<int64_t>::min(),
        std::numeric_limits<int64_t>::max());

    auto data = gen.generate_i64_constant(value);
    CHECK_TRUE(data.size() == 8);

    int64_t x = static_cast<int64_t>(data[0]) << 56;
    x |= static_cast<int64_t>(data[1]) << 48;
    x |= static_cast<int64_t>(data[2]) << 40;
    x |= static_cast<int64_t>(data[3]) << 32;
    x |= static_cast<int64_t>(data[4]) << 24;
    x |= static_cast<int64_t>(data[5]) << 16;
    x |= static_cast<int64_t>(data[6]) << 8;
    x |= static_cast<int64_t>(data[7]);
    CHECK_EQUAL(value, x);
  }

  //  fp
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    auto value = libutil::generate::generate_random_c<double>().get_range(
        std::numeric_limits<double>::min(),
        std::numeric_limits<double>::max());

    auto data = gen.generate_fp_constant(value);
    CHECK_TRUE(data.size() == 8);

    uint64_t x = static_cast<uint64_t>(data[0]) << 56;
    x |= static_cast<uint64_t>(data[1]) << 48;
    x |= static_cast<uint64_t>(data[2]) << 40;
    x |= static_cast<uint64_t>(data[3]) << 32;
    x |= static_cast<uint64_t>(data[4]) << 24;
    x |= static_cast<uint64_t>(data[5]) << 16;
    x |= static_cast<uint64_t>(data[6]) << 8;
    x |= static_cast<uint64_t>(data[7]);
    CHECK_EQUAL(value, libskiff::floating_point::from_uint64_t(x));
  }

  //  string
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    std::string s = libutil::generate::random_string_c().generate_string(
        libutil::generate::generate_random_c<uint8_t>().get_range(
          10, 20
        )
      );

    auto data = gen.gen_string_constant(s);

    CHECK_TRUE(data != std::nullopt);

    std::vector<uint8_t> value = data.value();

    uint16_t len = static_cast<uint16_t>(value[0]) >> 8;
    len |= static_cast<uint16_t>(value[1]);

    CHECK_EQUAL(s.size(), len);

    std::string result;
    for(std::size_t i = 2; i < value.size(); i++) {
      result += static_cast<char>(value[i]);
    }
    CHECK_EQUAL(s, result);
  }
}

TEST(instruction_generator_tests, registers)
{
  libskiff::instructions::instruction_generator_c gen;
  for(auto &el : known_good_registers) {
    auto value = gen.get_register_value(el.reg);
    CHECK_TRUE(value != std::nullopt);
    CHECK_EQUAL(el.value, *value);
  }

  for(auto &el : bad_values) {
    auto value = gen.get_register_value(el.reg);
    CHECK_TRUE(value != std::nullopt);
    CHECK_FALSE(el.value == *value);
  }

  for(auto i = 0; i < 10; i++) {
    auto value = gen.get_register_value(
      libutil::generate::random_string_c().generate_string(3)
    );
    CHECK_TRUE(value == std::nullopt);
  }
}

TEST(instruction_generator_tests, instruction_nop)
{
  libskiff::instructions::instruction_generator_c gen;
  auto bytes = gen.gen_nop();
  CHECK_EQUAL(libskiff::instructions::INS_SIZE_BYTES, bytes.size());
  for(auto i = 0; i < bytes.size(); i++) {
    if(i == 3) {
      CHECK_EQUAL_TEXT(libskiff::instructions::NOP, 
      bytes[i], 
      "Instruction opcode did not match expected value");
    } else {
      CHECK_EQUAL_TEXT(0x00, bytes[i], "Expected empty bytes");
    }
  }
}

TEST(instruction_generator_tests, instruction_exit)
{
  libskiff::instructions::instruction_generator_c gen;
  auto bytes = gen.gen_exit();
  CHECK_EQUAL(libskiff::instructions::INS_SIZE_BYTES, bytes.size());
  for(auto i = 0; i < bytes.size(); i++) {
    if(i == 3) {
      CHECK_EQUAL_TEXT(libskiff::instructions::EXIT, 
      bytes[i], 
      "Instruction opcode did not match expected value");
    } else {
      CHECK_EQUAL_TEXT(0x00, bytes[i], "Expected empty bytes");
    }
  }
}

TEST(instruction_generator_tests, instruction_branch)
{
  // All branch instructions match the same encoding, so we test them all here
  for(auto &ins: {
    libskiff::instructions::BLT,
    libskiff::instructions::BGT,
    libskiff::instructions::BEQ}) {
      
    auto lhs_register = known_good_registers[
      libutil::generate::generate_random_c<uint8_t>().get_range(0, 
        known_good_registers.size()-1)
    ];

    auto rhs_register = known_good_registers[
      libutil::generate::generate_random_c<uint8_t>().get_range(0, 
        known_good_registers.size()-1)
    ];

    auto address = libutil::generate::generate_random_c<uint32_t>().get_range(
      std::numeric_limits<uint32_t>::min(), 
      std::numeric_limits<uint32_t>::max()
    );

    libskiff::instructions::instruction_generator_c gen;

    std::vector<uint8_t> bytes;

    switch(ins) {
      case libskiff::instructions::BLT: 
        bytes = gen.gen_blt(lhs_register.value, rhs_register.value, address);
        break;
      case libskiff::instructions::BGT: 
        bytes = gen.gen_bgt(lhs_register.value, rhs_register.value, address);
        break;
      case libskiff::instructions::BEQ: 
        bytes = gen.gen_beq(lhs_register.value, rhs_register.value, address);
        break;
      default:
        FAIL("Default hit");
        break;
    };

    CHECK_EQUAL(libskiff::instructions::INS_SIZE_BYTES, bytes.size());

    CHECK_EQUAL_TEXT(0x00, bytes[0], "Expected empty byte");
    CHECK_EQUAL_TEXT(lhs_register.value, bytes[1], "Incorrect LHS register encoded");
    CHECK_EQUAL_TEXT(rhs_register.value, bytes[2], "Incorrect RHS register encoded");
        CHECK_EQUAL_TEXT(ins, 
        bytes[3], 
        "Instruction opcode did not match expected value");

    uint32_t decoded_address = 0;
    uint8_t shift = 24;
    for(auto i = 4; i < bytes.size(); i++) {
      decoded_address |= (static_cast<uint32_t>(bytes[i]) << shift);
      shift -= 8;
    }
    CHECK_EQUAL_TEXT(address, decoded_address, "Incorrect address encoded");
  }
}
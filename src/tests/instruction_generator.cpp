
#include <libskiff/bytecode/floating_point.hpp>
#include <libskiff/generator/instruction_generator.hpp>
#include <libutil/generate_random.hpp>
#include <libutil/random_string.hpp>

#include <bitset>
#include <iostream>

#include <CppUTest/TestHarness.h>

namespace {
struct tc_register_t {
  std::string reg;
  uint8_t value;
};

std::vector<tc_register_t> known_good_registers = {
    {"x0", 0x00}, {"x1", 0x01}, {"ip", 0x02}, {"sp", 0x03}, {"i0", 0x10},
    {"i1", 0x11}, {"i2", 0x12}, {"i3", 0x13}, {"i4", 0x14}, {"i5", 0x15},
    {"i6", 0x16}, {"i7", 0x17}, {"i8", 0x18}, {"i9", 0x19}, {"f0", 0x20},
    {"f1", 0x21}, {"f2", 0x22}, {"f3", 0x23}, {"f4", 0x24}, {"f5", 0x25},
    {"f6", 0x26}, {"f7", 0x27}, {"f8", 0x28}, {"f9", 0x29}, {"op", 0xFF}};

std::vector<tc_register_t> bad_values = {
    {"x0", 0x40}, {"x1", 0x41}, {"ip", 0x42}, {"sp", 0x43}, {"i0", 0x40},
    {"i1", 0x41}, {"i2", 0x42}, {"i3", 0x43}, {"i4", 0x44}, {"i5", 0x45},
    {"i6", 0x46}, {"i7", 0x47}, {"i8", 0x48}, {"i9", 0x40}, {"f0", 0x40},
    {"f1", 0x41}, {"f2", 0x42}, {"f3", 0x43}, {"f4", 0x44}, {"f5", 0x45},
    {"f6", 0x46}, {"f7", 0x47}, {"f8", 0x48}, {"f9", 0x40},
};
} // namespace

TEST_GROUP(instruction_generator_tests){};

TEST(instruction_generator_tests, constants)
{
  constexpr uint8_t num_const_tests = 10;
  libskiff::generator::instruction_generator_c gen;

  //  .u8
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    auto value = libutil::generate::generate_random_c<uint8_t>().get_range(
        std::numeric_limits<uint8_t>::min(),
        std::numeric_limits<uint8_t>::max());

    auto data = gen.generate_u8_constant(value);
    CHECK_TRUE(data.size() == 2);
    uint16_t x = static_cast<uint16_t>(data[0]) << 8;
    x |= static_cast<uint16_t>(data[1]);
    CHECK_EQUAL(value, x);
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
        std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max());

    auto data = gen.generate_i8_constant(value);
    CHECK_TRUE(data.size() == 2);
    int16_t x = static_cast<int16_t>(data[0]) << 8;
    x |= static_cast<int16_t>(data[1]);
    CHECK_EQUAL(value, static_cast<int8_t>(x));
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
    auto value =
        libutil::generate::generate_random_c<double>().get_floating_point_range(
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
    CHECK_EQUAL(value, libskiff::bytecode::floating_point::from_uint64_t(x));
  }

  //  string
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    std::string s = libutil::generate::random_string_c().generate_string(
        libutil::generate::generate_random_c<uint8_t>().get_range(10, 20));

    // Word align the string
    if (s.size() % 2 != 0) {
      s += static_cast<char>(0x00);
    }

    auto data = gen.gen_string_constant(s);

    CHECK_TRUE(data != std::nullopt);

    std::vector<uint8_t> value = data.value();

    CHECK_EQUAL(s.size() + 8, value.size());

    uint64_t len = static_cast<uint64_t>(value[0]) << 56;
    len |= static_cast<uint64_t>(value[1]) << 48;
    len |= static_cast<uint64_t>(value[2]) << 40;
    len |= static_cast<uint64_t>(value[3]) << 32;
    len |= static_cast<uint64_t>(value[4]) << 24;
    len |= static_cast<uint64_t>(value[5]) << 16;
    len |= static_cast<uint64_t>(value[6]) << 8;
    len |= static_cast<uint64_t>(value[7]);

    CHECK_EQUAL(s.size(), len);

    std::string result;
    // Skip the first pad, and then the char should be
    // every other byte
    for (std::size_t i = 8; i < value.size(); i++) {
      result += static_cast<char>(value[i]);
    }
    CHECK_EQUAL(s, result);
  }

  //  string - no char padding
  //
  for (uint8_t i = 0; i < num_const_tests; i++) {
    std::string s = libutil::generate::random_string_c().generate_string(
        libutil::generate::generate_random_c<uint8_t>().get_range(10, 20));

    // Word align the string
    if (s.size() % 2 != 0) {
      s += static_cast<char>(0x00);
    }

    auto data = gen.gen_string_constant(s);

    CHECK_TRUE(data != std::nullopt);

    std::vector<uint8_t> value = data.value();

    CHECK_EQUAL(s.size() + 8, value.size());

    uint64_t len = static_cast<uint64_t>(value[0]) << 56;
    len |= static_cast<uint64_t>(value[1]) << 48;
    len |= static_cast<uint64_t>(value[2]) << 40;
    len |= static_cast<uint64_t>(value[3]) << 32;
    len |= static_cast<uint64_t>(value[4]) << 24;
    len |= static_cast<uint64_t>(value[5]) << 16;
    len |= static_cast<uint64_t>(value[6]) << 8;
    len |= static_cast<uint64_t>(value[7]);

    CHECK_EQUAL(s.size(), len);

    std::string result;
    for (std::size_t i = 8; i < value.size(); i++) {
      result += static_cast<char>(value[i]);
    }
    CHECK_EQUAL(s, result);
  }
}

TEST(instruction_generator_tests, lib_sections)
{
  libskiff::generator::instruction_generator_c gen;
  for (uint8_t i = 0; i < 10; i++) {
    std::string s = libutil::generate::random_string_c().generate_string(
        libutil::generate::generate_random_c<uint8_t>().get_range(10, 20));

    // Word align the string
    if (s.size() % 2 != 0) {
      s += static_cast<char>(0x00);
    }

    uint64_t address =
        libutil::generate::generate_random_c<uint64_t>().get_range(
            std::numeric_limits<uint64_t>::min(),
            std::numeric_limits<uint64_t>::max());

    auto encoded = gen.gen_lib_section(address, s);

    CHECK_TRUE_TEXT(encoded != std::nullopt,
                    "Unable to encode string into library section");

    auto value = encoded.value();

    CHECK_EQUAL(s.size() + 16, value.size());

    uint64_t decoded_addr = static_cast<uint64_t>(value[0]) << 56;
    decoded_addr |= static_cast<uint64_t>(value[1]) << 48;
    decoded_addr |= static_cast<uint64_t>(value[2]) << 40;
    decoded_addr |= static_cast<uint64_t>(value[3]) << 32;
    decoded_addr |= static_cast<uint64_t>(value[4]) << 24;
    decoded_addr |= static_cast<uint64_t>(value[5]) << 16;
    decoded_addr |= static_cast<uint64_t>(value[6]) << 8;
    decoded_addr |= static_cast<uint64_t>(value[7]);

    CHECK_EQUAL(address, decoded_addr);

    uint64_t len = static_cast<uint64_t>(value[8]) << 56;
    len |= static_cast<uint64_t>(value[9]) << 48;
    len |= static_cast<uint64_t>(value[10]) << 40;
    len |= static_cast<uint64_t>(value[11]) << 32;
    len |= static_cast<uint64_t>(value[12]) << 24;
    len |= static_cast<uint64_t>(value[13]) << 16;
    len |= static_cast<uint64_t>(value[14]) << 8;
    len |= static_cast<uint64_t>(value[15]);

    CHECK_EQUAL(s.size(), len);

    std::string result;
    for (std::size_t i = 16; i < value.size(); i++) {
      result += static_cast<char>(value[i]);
    }
    CHECK_EQUAL(s, result);
  }
}

TEST(instruction_generator_tests, registers)
{
  libskiff::generator::instruction_generator_c gen;
  for (auto &el : known_good_registers) {
    auto value = gen.get_register_value(el.reg);
    CHECK_TRUE(value != std::nullopt);
    CHECK_EQUAL(el.value, *value);
  }

  for (auto &el : bad_values) {
    auto value = gen.get_register_value(el.reg);
    CHECK_TRUE(value != std::nullopt);
    CHECK_FALSE(el.value == *value);
  }

  for (auto i = 0; i < 10; i++) {
    auto value = gen.get_register_value(
        libutil::generate::random_string_c().generate_string(3));
    CHECK_TRUE(value == std::nullopt);
  }
}

TEST(instruction_generator_tests, instruction_nop)
{
  // Check size map setup test
  uint8_t expected_instruction_size_bytes = 1;
  auto expected_instruction = libskiff::bytecode::instructions::NOP;
  auto size_map =
      libskiff::bytecode::instructions::get_instruction_to_size_map();
  CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                  "Expected instruction not present in size map");
  CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                   size_map[expected_instruction],
                   "Incorrect size in size map for given instruction");

  // Check instruction encoding
  libskiff::generator::instruction_generator_c gen;

  auto bytes = gen.gen_nop();
  CHECK_EQUAL(expected_instruction_size_bytes, bytes.size());
  CHECK_EQUAL_TEXT(expected_instruction, bytes[0], "Unexpected opcode");
}

TEST(instruction_generator_tests, instruction_exit)
{
  // Check size map setup test
  uint8_t expected_instruction_size_bytes = 1;
  auto expected_instruction = libskiff::bytecode::instructions::EXIT;
  auto size_map =
      libskiff::bytecode::instructions::get_instruction_to_size_map();
  CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                  "Expected instruction not present in size map");
  CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                   size_map[expected_instruction],
                   "Incorrect size in size map for given instruction");

  // Check instruction encoding
  libskiff::generator::instruction_generator_c gen;

  auto bytes = gen.gen_exit();
  CHECK_EQUAL(expected_instruction_size_bytes, bytes.size());
  CHECK_EQUAL_TEXT(expected_instruction, bytes[0], "Unexpected opcode");
}

TEST(instruction_generator_tests, instruction_branch)
{
  // All branch instructions match the same encoding, so we test them all here
  for (auto &ins : {libskiff::bytecode::instructions::BLT,
                    libskiff::bytecode::instructions::BGT,
                    libskiff::bytecode::instructions::BEQ,
                    libskiff::bytecode::instructions::BLTF,
                    libskiff::bytecode::instructions::BGTF,
                    libskiff::bytecode::instructions::BEQF}) {

    // Check size map setup test
    uint8_t expected_instruction_size_bytes = 11;
    auto expected_instruction = ins;
    auto size_map =
        libskiff::bytecode::instructions::get_instruction_to_size_map();
    CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                    "Expected instruction not present in size map");
    CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                     size_map[expected_instruction],
                     "Incorrect size in size map for given instruction");

    auto lhs_register =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];

    auto rhs_register =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];

    auto address = libutil::generate::generate_random_c<uint64_t>().get_range(
        std::numeric_limits<uint64_t>::min(),
        std::numeric_limits<uint64_t>::max());

    libskiff::generator::instruction_generator_c gen;

    std::vector<uint8_t> bytes;

    switch (ins) {
    case libskiff::bytecode::instructions::BLT:
      bytes = gen.gen_blt(lhs_register.value, rhs_register.value, address);
      break;
    case libskiff::bytecode::instructions::BGT:
      bytes = gen.gen_bgt(lhs_register.value, rhs_register.value, address);
      break;
    case libskiff::bytecode::instructions::BEQ:
      bytes = gen.gen_beq(lhs_register.value, rhs_register.value, address);
      break;
    case libskiff::bytecode::instructions::BLTF:
      bytes = gen.gen_bltf(lhs_register.value, rhs_register.value, address);
      break;
    case libskiff::bytecode::instructions::BGTF:
      bytes = gen.gen_bgtf(lhs_register.value, rhs_register.value, address);
      break;
    case libskiff::bytecode::instructions::BEQF:
      bytes = gen.gen_beqf(lhs_register.value, rhs_register.value, address);
      break;
    default:
      FAIL("Default hit");
      break;
    };

    CHECK_EQUAL(expected_instruction_size_bytes, bytes.size());

    CHECK_EQUAL_TEXT(ins, bytes[0],
                     "Instruction opcode did not match expected value");
    CHECK_EQUAL_TEXT(lhs_register.value, bytes[1],
                     "Incorrect LHS register encoded");
    CHECK_EQUAL_TEXT(rhs_register.value, bytes[2],
                     "Incorrect RHS register encoded");

    uint64_t decoded_address = 0;
    uint8_t shift = 56;
    for (auto i = 3; i < bytes.size(); i++) {
      decoded_address |= (static_cast<uint64_t>(bytes[i]) << shift);
      shift -= 8;
    }
    CHECK_EQUAL_TEXT(address, decoded_address, "Incorrect address encoded");
  }
}

TEST(instruction_generator_tests, instruction_not)
{
  // Check size map setup test
  uint8_t expected_instruction_size_bytes = 3;
  auto expected_instruction = libskiff::bytecode::instructions::NOT;
  auto size_map =
      libskiff::bytecode::instructions::get_instruction_to_size_map();
  CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                  "Expected instruction not present in size map");
  CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                   size_map[expected_instruction],
                   "Incorrect size in size map for given instruction");

  auto dest =
      known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                               .get_range(0, known_good_registers.size() - 1)];

  auto source =
      known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                               .get_range(0, known_good_registers.size() - 1)];

  libskiff::generator::instruction_generator_c gen;
  auto bytes = gen.gen_not(dest.value, source.value);

  CHECK_EQUAL_TEXT(expected_instruction, bytes[0], "Unexpected opcode");
  CHECK_EQUAL_TEXT(dest.value, bytes[1], "Unexpected byte");
  CHECK_EQUAL_TEXT(source.value, bytes[2], "Unexpected byte");
}

TEST(instruction_generator_tests, instruction_ret)
{
  // Check size map setup test
  uint8_t expected_instruction_size_bytes = 1;
  auto expected_instruction = libskiff::bytecode::instructions::RET;
  auto size_map =
      libskiff::bytecode::instructions::get_instruction_to_size_map();
  CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                  "Expected instruction not present in size map");
  CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                   size_map[expected_instruction],
                   "Incorrect size in size map for given instruction");

  // Check instruction encoding
  libskiff::generator::instruction_generator_c gen;

  auto bytes = gen.gen_ret();
  CHECK_EQUAL(expected_instruction_size_bytes, bytes.size());
  CHECK_EQUAL_TEXT(expected_instruction, bytes[0], "Unexpected opcode");
}

TEST(instruction_generator_tests, instruction_mov)
{
  // Check size map setup test
  uint8_t expected_instruction_size_bytes = 10;
  auto expected_instruction = libskiff::bytecode::instructions::MOV;
  auto size_map =
      libskiff::bytecode::instructions::get_instruction_to_size_map();
  CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                  "Expected instruction not present in size map");
  CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                   size_map[expected_instruction],
                   "Incorrect size in size map for given instruction");

  auto dest_register =
      known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                               .get_range(0, known_good_registers.size() - 1)];

  auto value = libutil::generate::generate_random_c<uint64_t>().get_range(
      std::numeric_limits<uint64_t>::min(),
      std::numeric_limits<uint64_t>::max());

  libskiff::generator::instruction_generator_c gen;
  auto bytes = gen.gen_mov(dest_register.value, value);

  CHECK_EQUAL_TEXT(libskiff::bytecode::instructions::MOV, bytes[0],
                   "Instruction opcode did not match expected value");
  CHECK_EQUAL_TEXT(dest_register.value, bytes[1],
                   "Incorrect destination register encoded");

  uint64_t decoded_value = 0;
  uint8_t shift = 56;
  for (auto i = 2; i < bytes.size(); i++) {
    decoded_value |= (static_cast<uint64_t>(bytes[i]) << shift);
    shift -= 8;
  }
  CHECK_EQUAL_TEXT(value, decoded_value, "Incorrect address encoded");
}

TEST(instruction_generator_tests, instruction_math_ops)
{
  // All of these math instructions match the same encoding, so we test them all
  // here
  for (auto &ins : {libskiff::bytecode::instructions::ADD,
                    libskiff::bytecode::instructions::ADDF,
                    libskiff::bytecode::instructions::SUB,
                    libskiff::bytecode::instructions::SUBF,
                    libskiff::bytecode::instructions::DIV,
                    libskiff::bytecode::instructions::DIVF,
                    libskiff::bytecode::instructions::MUL,
                    libskiff::bytecode::instructions::MULF,
                    libskiff::bytecode::instructions::LSH,
                    libskiff::bytecode::instructions::RSH,
                    libskiff::bytecode::instructions::AND,
                    libskiff::bytecode::instructions::OR,
                    libskiff::bytecode::instructions::XOR}) {

    // Check size map setup test
    uint8_t expected_instruction_size_bytes = 4;
    auto expected_instruction = ins;
    auto size_map =
        libskiff::bytecode::instructions::get_instruction_to_size_map();
    CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                    "Expected instruction not present in size map");
    CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                     size_map[expected_instruction],
                     "Incorrect size in size map for given instruction");

    auto dest_register =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];

    auto lhs_register =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];

    auto rhs_register =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];

    libskiff::generator::instruction_generator_c gen;

    std::vector<uint8_t> bytes;

    switch (ins) {
    case libskiff::bytecode::instructions::ADD:
      bytes = gen.gen_add(dest_register.value, lhs_register.value,
                          rhs_register.value);
      break;
    case libskiff::bytecode::instructions::ADDF:
      bytes = gen.gen_addf(dest_register.value, lhs_register.value,
                           rhs_register.value);
      break;
    case libskiff::bytecode::instructions::SUB:
      bytes = gen.gen_sub(dest_register.value, lhs_register.value,
                          rhs_register.value);
      break;
    case libskiff::bytecode::instructions::SUBF:
      bytes = gen.gen_subf(dest_register.value, lhs_register.value,
                           rhs_register.value);
      break;
    case libskiff::bytecode::instructions::DIV:
      bytes = gen.gen_div(dest_register.value, lhs_register.value,
                          rhs_register.value);
      break;
    case libskiff::bytecode::instructions::DIVF:
      bytes = gen.gen_divf(dest_register.value, lhs_register.value,
                           rhs_register.value);
      break;
    case libskiff::bytecode::instructions::MUL:
      bytes = gen.gen_mul(dest_register.value, lhs_register.value,
                          rhs_register.value);
      break;
    case libskiff::bytecode::instructions::MULF:
      bytes = gen.gen_mulf(dest_register.value, lhs_register.value,
                           rhs_register.value);
      break;
    case libskiff::bytecode::instructions::LSH:
      bytes = gen.gen_lsh(dest_register.value, lhs_register.value,
                          rhs_register.value);
      break;
    case libskiff::bytecode::instructions::RSH:
      bytes = gen.gen_rsh(dest_register.value, lhs_register.value,
                          rhs_register.value);
      break;
    case libskiff::bytecode::instructions::AND:
      bytes = gen.gen_and(dest_register.value, lhs_register.value,
                          rhs_register.value);
      break;
    case libskiff::bytecode::instructions::OR:
      bytes = gen.gen_or(dest_register.value, lhs_register.value,
                         rhs_register.value);
      break;
    case libskiff::bytecode::instructions::XOR:
      bytes = gen.gen_xor(dest_register.value, lhs_register.value,
                          rhs_register.value);
      break;
    default:
      FAIL("Default hit");
      break;
    };
    CHECK_EQUAL_TEXT(ins, bytes[0], "Unexpected opcode");
    CHECK_EQUAL_TEXT(dest_register.value, bytes[1], "Unexpected byte");
    CHECK_EQUAL_TEXT(lhs_register.value, bytes[2], "Unexpected byte");
    CHECK_EQUAL_TEXT(rhs_register.value, bytes[3], "Unexpected byte");
  }
}

TEST(instruction_generator_tests, instruction_asserts)
{
  // All of these math instructions match the same encoding, so we test them all
  // here
  for (auto &ins : {libskiff::bytecode::instructions::ASEQ,
                    libskiff::bytecode::instructions::ASNE}) {

    // Check size map setup test
    uint8_t expected_instruction_size_bytes = 3;
    auto expected_instruction = ins;
    auto size_map =
        libskiff::bytecode::instructions::get_instruction_to_size_map();
    CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                    "Expected instruction not present in size map");
    CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                     size_map[expected_instruction],
                     "Incorrect size in size map for given instruction");

    auto expected =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];

    auto actual =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];

    libskiff::generator::instruction_generator_c gen;

    std::vector<uint8_t> bytes;

    switch (ins) {
    case libskiff::bytecode::instructions::ASEQ:
      bytes = gen.gen_aseq(expected.value, actual.value);
      break;
    case libskiff::bytecode::instructions::ASNE:
      bytes = gen.gen_asne(expected.value, actual.value);
      break;
    default:
      FAIL("Default hit");
      break;
    };

    CHECK_EQUAL_TEXT(ins, bytes[0], "Unexpected opcode");
    CHECK_EQUAL_TEXT(expected.value, bytes[1], "Unexpected byte");
    CHECK_EQUAL_TEXT(actual.value, bytes[2], "Unexpected");
  }
}

TEST(instruction_generator_tests, instruction_stack_pushpop)
{
  for (auto &ins : {
           libskiff::bytecode::instructions::PUSH_W,
           libskiff::bytecode::instructions::PUSH_DW,
           libskiff::bytecode::instructions::PUSH_QW,
           libskiff::bytecode::instructions::POP_W,
           libskiff::bytecode::instructions::POP_DW,
           libskiff::bytecode::instructions::POP_QW,

       }) {

    // Check size map setup test
    uint8_t expected_instruction_size_bytes = 2;
    auto expected_instruction = ins;
    auto size_map =
        libskiff::bytecode::instructions::get_instruction_to_size_map();
    CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                    "Expected instruction not present in size map");
    CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                     size_map[expected_instruction],
                     "Incorrect size in size map for given instruction");

    auto reg =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];

    libskiff::generator::instruction_generator_c gen;

    std::vector<uint8_t> bytes;
    switch (ins) {
    case libskiff::bytecode::instructions::PUSH_W:
      bytes = gen.gen_push_w(reg.value);
      break;
    case libskiff::bytecode::instructions::PUSH_DW:
      bytes = gen.gen_push_dw(reg.value);
      break;
    case libskiff::bytecode::instructions::PUSH_QW:
      bytes = gen.gen_push_qw(reg.value);
      break;
    case libskiff::bytecode::instructions::POP_W:
      bytes = gen.gen_pop_w(reg.value);
      break;
    case libskiff::bytecode::instructions::POP_DW:
      bytes = gen.gen_pop_dw(reg.value);
      break;
    case libskiff::bytecode::instructions::POP_QW:
      bytes = gen.gen_pop_qw(reg.value);
      break;
    };

    CHECK_EQUAL_TEXT(reg.value, bytes[1], "Unexpected byte");
    CHECK_EQUAL_TEXT(ins, bytes[0], "Unexpected opcode");
  }
}

TEST(instruction_generator_tests, instruction_alloc)
{
  // Check size map setup test
  uint8_t expected_instruction_size_bytes = 3;
  auto expected_instruction = libskiff::bytecode::instructions::ALLOC;
  auto size_map =
      libskiff::bytecode::instructions::get_instruction_to_size_map();
  CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                  "Expected instruction not present in size map");
  CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                   size_map[expected_instruction],
                   "Incorrect size in size map for given instruction");

  auto dest_register =
      known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                               .get_range(0, known_good_registers.size() - 1)];

  auto source_register =
      known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                               .get_range(0, known_good_registers.size() - 1)];

  libskiff::generator::instruction_generator_c gen;
  auto bytes = gen.gen_alloc(dest_register.value, source_register.value);

  CHECK_EQUAL_TEXT(expected_instruction, bytes[0], "Unexpected opcode");
  CHECK_EQUAL_TEXT(dest_register.value, bytes[1], "Unexpected byte");
  CHECK_EQUAL_TEXT(source_register.value, bytes[2], "Unexpected byte");
}

TEST(instruction_generator_tests, instruction_free)
{
  // Check size map setup test
  uint8_t expected_instruction_size_bytes = 2;
  auto expected_instruction = libskiff::bytecode::instructions::FREE;
  auto size_map =
      libskiff::bytecode::instructions::get_instruction_to_size_map();
  CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                  "Expected instruction not present in size map");
  CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                   size_map[expected_instruction],
                   "Incorrect size in size map for given instruction");

  auto index_register =
      known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                               .get_range(0, known_good_registers.size() - 1)];

  libskiff::generator::instruction_generator_c gen;
  auto bytes = gen.gen_free(index_register.value);

  CHECK_EQUAL_TEXT(index_register.value, bytes[1], "Unexpected byte");
  CHECK_EQUAL_TEXT(expected_instruction, bytes[0], "Unexpected opcode");
}

TEST(instruction_generator_tests, instructions_load_store)
{
  for (auto &ins : {
           libskiff::bytecode::instructions::SW,
           libskiff::bytecode::instructions::SDW,
           libskiff::bytecode::instructions::SQW,
           libskiff::bytecode::instructions::LW,
           libskiff::bytecode::instructions::LDW,
           libskiff::bytecode::instructions::LQW,

       }) {

    // Check size map setup test
    uint8_t expected_instruction_size_bytes = 4;
    auto expected_instruction = ins;
    auto size_map =
        libskiff::bytecode::instructions::get_instruction_to_size_map();
    CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                    "Expected instruction not present in size map");
    CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                     size_map[expected_instruction],
                     "Incorrect size in size map for given instruction");

    auto idx =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];
    auto offset =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];
    auto data =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];

    libskiff::generator::instruction_generator_c gen;

    std::vector<uint8_t> bytes;
    switch (ins) {
    case libskiff::bytecode::instructions::SW:
      bytes = gen.gen_store_word(idx.value, offset.value, data.value);
      break;
    case libskiff::bytecode::instructions::SDW:
      bytes = gen.gen_store_dword(idx.value, offset.value, data.value);
      break;
    case libskiff::bytecode::instructions::SQW:
      bytes = gen.gen_store_qword(idx.value, offset.value, data.value);
      break;
    case libskiff::bytecode::instructions::LW:
      bytes = gen.gen_load_word(idx.value, offset.value, data.value);
      break;
    case libskiff::bytecode::instructions::LDW:
      bytes = gen.gen_load_dword(idx.value, offset.value, data.value);
      break;
    case libskiff::bytecode::instructions::LQW:
      bytes = gen.gen_load_qword(idx.value, offset.value, data.value);
      break;
    };

    CHECK_EQUAL_TEXT(expected_instruction, bytes[0], "Unexpected opcode");
    CHECK_EQUAL_TEXT(idx.value, bytes[1], "Unexpected byte");
    CHECK_EQUAL_TEXT(offset.value, bytes[2], "Unexpected byte");
    CHECK_EQUAL_TEXT(data.value, bytes[3], "Unexpected byte");
  }
}

TEST(instruction_generator_tests, instructions_address_based)
{
  for (auto &ins : {
           libskiff::bytecode::instructions::SYSCALL,
           libskiff::bytecode::instructions::CALL,
           libskiff::bytecode::instructions::JMP,
           libskiff::bytecode::instructions::DEBUG,
       }) {

    // Check size map setup test
    uint8_t expected_instruction_size_bytes = 9;
    auto expected_instruction = ins;
    auto size_map =
        libskiff::bytecode::instructions::get_instruction_to_size_map();
    CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                    "Expected instruction not present in size map");
    CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                     size_map[expected_instruction],
                     "Incorrect size in size map for given instruction");

    auto address = libutil::generate::generate_random_c<uint64_t>().get_range(
        std::numeric_limits<uint64_t>::min(),
        std::numeric_limits<uint64_t>::max());

    libskiff::generator::instruction_generator_c gen;

    std::vector<uint8_t> bytes;
    switch (ins) {
    case libskiff::bytecode::instructions::SYSCALL:
      bytes = gen.gen_syscall(address);
      break;
    case libskiff::bytecode::instructions::JMP:
      bytes = gen.gen_jmp(address);
      break;
    case libskiff::bytecode::instructions::CALL:
      bytes = gen.gen_call(address);
      break;
    case libskiff::bytecode::instructions::DEBUG:
      bytes = gen.gen_debug(address);
      break;
    };

    CHECK_EQUAL_TEXT(expected_instruction, bytes[0], "Unexpected opcode");

    uint64_t decoded_address = 0;
    uint8_t shift = 56;
    for (auto i = 1; i < bytes.size(); i++) {
      decoded_address |= (static_cast<uint64_t>(bytes[i]) << shift);
      shift -= 8;
    }
    CHECK_EQUAL_TEXT(address, decoded_address, "Incorrect address encoded");
  }
}

TEST(instruction_generator_tests, instruction_irqs)
{
  for (auto &ins : {
           libskiff::bytecode::instructions::EIRQ,
           libskiff::bytecode::instructions::DIRQ,
       }) {

    // Check size map setup test
    uint8_t expected_instruction_size_bytes = 1;
    auto expected_instruction = ins;
    auto size_map =
        libskiff::bytecode::instructions::get_instruction_to_size_map();
    CHECK_TRUE_TEXT(size_map.find(expected_instruction) != size_map.end(),
                    "Expected instruction not present in size map");
    CHECK_EQUAL_TEXT(expected_instruction_size_bytes,
                     size_map[expected_instruction],
                     "Incorrect size in size map for given instruction");

    auto reg =
        known_good_registers[libutil::generate::generate_random_c<uint8_t>()
                                 .get_range(0,
                                            known_good_registers.size() - 1)];

    libskiff::generator::instruction_generator_c gen;

    std::vector<uint8_t> bytes;
    switch (ins) {
    case libskiff::bytecode::instructions::EIRQ:
      bytes = gen.gen_eirq();
      break;
    case libskiff::bytecode::instructions::DIRQ:
      bytes = gen.gen_dirq();
      break;
    };

    CHECK_EQUAL_TEXT(ins, bytes[0], "Unexpected opcode");
  }
}
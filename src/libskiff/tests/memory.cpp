#include <libskiff/machine/memory/memory.hpp>
#include <libskiff/system.hpp>
#include <libutil/generate_random.hpp>

#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

#include <CppUTest/TestHarness.h>

namespace {

static constexpr uint8_t num_tests_per_type = 100;

template <typename T> static std::vector<T> generate_tc()
{
  std::vector<T> tc;
  tc.reserve(num_tests_per_type);
  for (auto i = 0; i < num_tests_per_type; i++) {
    tc.push_back(libutil::generate::generate_random_c<T>().get_range(
        std::numeric_limits<T>::min(), std::numeric_limits<T>::max()));
  }
  return tc;
}
} // namespace

TEST_GROUP(memory_c){};

TEST(memory_c, mem_words)
{
  libskiff::machine::memory::memory_c skiff_memory(libskiff::system::word_size_bytes *
                                           num_tests_per_type * 2);
  auto test_case = generate_tc<uint16_t>();
  uint64_t idx = 0;
  for (auto &tc : test_case) {
    CHECK_TRUE_TEXT(skiff_memory.put_word(idx, tc), "Unable to put word");
    idx += libskiff::system::word_size_bytes;
  }

  idx = 0;
  for (auto &tc : test_case) {
    auto [okay, value] = skiff_memory.get_word(idx);
    CHECK_TRUE_TEXT(okay, "No success getting word");
    CHECK_EQUAL_TEXT(tc, value, "Value did not meet expectations");
    idx += libskiff::system::word_size_bytes;
  }
}

TEST(memory_c, mem_dwords)
{
  libskiff::machine::memory::memory_c skiff_memory(libskiff::system::d_word_size_bytes *
                                           num_tests_per_type * 2);
  auto test_case = generate_tc<uint32_t>();
  uint64_t idx = 0;
  for (auto &tc : test_case) {
    CHECK_TRUE_TEXT(skiff_memory.put_dword(idx, tc), "Unable to put dword");
    idx += libskiff::system::d_word_size_bytes;
  }

  idx = 0;
  for (auto &tc : test_case) {
    auto [okay, value] = skiff_memory.get_dword(idx);
    CHECK_TRUE_TEXT(okay, "No success getting dword");
    CHECK_EQUAL_TEXT(tc, value, "Value did not meet expectations");
    idx += libskiff::system::d_word_size_bytes;
  }
}

TEST(memory_c, mem_qwords)
{
  libskiff::machine::memory::memory_c skiff_memory(libskiff::system::q_word_size_bytes *
                                           num_tests_per_type * 2);
  auto test_case = generate_tc<uint64_t>();
  uint64_t idx = 0;
  for (auto &tc : test_case) {
    CHECK_TRUE_TEXT(skiff_memory.put_qword(idx, tc), "Unable to put qword");
    idx += libskiff::system::q_word_size_bytes;
  }

  idx = 0;
  for (auto &tc : test_case) {
    auto [okay, value] = skiff_memory.get_qword(idx);
    CHECK_TRUE_TEXT(okay, "No success getting qword");
    CHECK_EQUAL_TEXT(tc, value, "Value did not meet expectations");
    idx += libskiff::system::q_word_size_bytes;
  }
}

TEST(memory_c, chonker)
{
  /*
      This test generates a list of size num_tests_per_type * 3,
      and populates with random (WORD, DWORD, and QWORD) items.

      Every item is added to the stack and then removed. This will
      ensure that there are no issues with "mixed" types within the stack.
  */
  enum class type_t { WORD = 0, DWORD = 1, QWORD = 2 };

  struct tc {
    type_t type;
    uint64_t value;
  };

  //  Build some test cases
  //
  std::vector<tc> test_cases;
  for (auto i = 0; i < num_tests_per_type * 3; i++) {
    tc test_case;
    test_case.type = static_cast<type_t>(
        libutil::generate::generate_random_c<uint8_t>().get_range(0, 2));
    switch (test_case.type) {
    case type_t::WORD:
      test_case.value = static_cast<uint64_t>(
          libutil::generate::generate_random_c<uint16_t>().get_range(
              std::numeric_limits<uint16_t>::min(),
              std::numeric_limits<uint16_t>::max()));
      break;
    case type_t::DWORD:
      test_case.value = static_cast<uint64_t>(
          libutil::generate::generate_random_c<uint32_t>().get_range(
              std::numeric_limits<uint32_t>::min(),
              std::numeric_limits<uint32_t>::max()));
      break;
    case type_t::QWORD:
      test_case.value =
          libutil::generate::generate_random_c<uint64_t>().get_range(
              std::numeric_limits<uint64_t>::min(),
              std::numeric_limits<uint64_t>::max());
      break;
    default:
      FAIL("Random number gen broke the freakin tests");
      break;
    };
    test_cases.push_back(test_case);
  }

  libskiff::machine::memory::memory_c skiff_memory(
      num_tests_per_type * libskiff::system::q_word_size_bytes * 3);

  // Insert all cases
  uint64_t idx = 0;
  for (auto &i : test_cases) {
    switch (i.type) {
    case type_t::WORD:
      CHECK_TRUE(skiff_memory.put_word(idx, static_cast<uint16_t>(i.value)));
      idx += libskiff::system::word_size_bytes;
      break;
    case type_t::DWORD:
      CHECK_TRUE(skiff_memory.put_dword(idx, static_cast<uint32_t>(i.value)));
      idx += libskiff::system::d_word_size_bytes;
      break;
    case type_t::QWORD:
      CHECK_TRUE(skiff_memory.put_qword(idx, i.value));
      idx += libskiff::system::q_word_size_bytes;
      break;
    default:
      FAIL("Random number gen broke the freakin tests");
      break;
    };
  }

  idx = 0;
  for (auto &i : test_cases) {
    switch (i.type) {
    case type_t::WORD: {
      auto [okay, value] = skiff_memory.get_word(idx);
      CHECK_TRUE(okay);
      CHECK_EQUAL(i.value, static_cast<uint64_t>(value));
      idx += libskiff::system::word_size_bytes;
    } break;
    case type_t::DWORD: {
      auto [okay, value] = skiff_memory.get_dword(idx);
      CHECK_TRUE(okay);
      CHECK_EQUAL(i.value, static_cast<uint64_t>(value));
      idx += libskiff::system::d_word_size_bytes;
    } break;
    case type_t::QWORD: {
      auto [okay, value] = skiff_memory.get_qword(idx);
      CHECK_TRUE(okay);
      CHECK_EQUAL(i.value, value);
      idx += libskiff::system::q_word_size_bytes;
    } break;
    default:
      FAIL("Random number gen broke the freakin tests");
      break;
    }
  }
}
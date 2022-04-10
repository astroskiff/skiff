
#include <libskiff/machine/memory/stack.hpp>
#include <libskiff/system.hpp>
#include <libskiff/types.hpp>
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

TEST_GROUP(memory_stack){};

TEST(memory_stack, words)
{
  libskiff::types::vm_register stack_pointer = 0;
  libskiff::machine::memory::stack_c skiff_stack;
  skiff_stack.set_sp(stack_pointer);
  libskiff::types::vm_register expected_sp = 0;

  auto test_case = generate_tc<uint16_t>();
  for (auto &tc : test_case) {
    expected_sp += libskiff::system::word_size_bytes;
    CHECK_TRUE_TEXT(skiff_stack.push_word(tc), "Unable to push word");
    CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
  }

  for (auto i = test_case.rbegin(); i != test_case.rend(); i++) {
    auto [okay, value] = skiff_stack.pop_word();
    expected_sp -= libskiff::system::word_size_bytes;
    CHECK_TRUE_TEXT(okay, "No success popping stack");
    CHECK_EQUAL_TEXT((*i), value, "Value did not meet expectations");
    CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
  }
}

TEST(memory_stack, d_words)
{
  libskiff::types::vm_register stack_pointer = 0;
  libskiff::machine::memory::stack_c skiff_stack;
  skiff_stack.set_sp(stack_pointer);
  libskiff::types::vm_register expected_sp = 0;

  auto test_case = generate_tc<uint32_t>();
  for (auto &tc : test_case) {
    expected_sp += libskiff::system::d_word_size_bytes;
    CHECK_TRUE_TEXT(skiff_stack.push_dword(tc), "Unable to push dword");
    CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
  }

  for (auto i = test_case.rbegin(); i != test_case.rend(); i++) {
    auto [okay, value] = skiff_stack.pop_dword();
    expected_sp -= libskiff::system::d_word_size_bytes;
    CHECK_TRUE_TEXT(okay, "No success popping stack");
    CHECK_EQUAL_TEXT((*i), value, "Value did not meet expectations");
    CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
  }
}

TEST(memory_stack, q_words)
{
  libskiff::types::vm_register stack_pointer = 0;
  libskiff::machine::memory::stack_c skiff_stack;
  skiff_stack.set_sp(stack_pointer);
  libskiff::types::vm_register expected_sp = 0;

  auto test_case = generate_tc<uint64_t>();
  for (auto &tc : test_case) {
    expected_sp += libskiff::system::q_word_size_bytes;
    CHECK_TRUE_TEXT(skiff_stack.push_qword(tc), "Unable to push qword");
    CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
  }

  for (auto i = test_case.rbegin(); i != test_case.rend(); i++) {
    auto [okay, value] = skiff_stack.pop_qword();
    expected_sp -= libskiff::system::q_word_size_bytes;
    CHECK_TRUE_TEXT(okay, "No success popping stack");
    CHECK_EQUAL_TEXT((*i), value, "Value did not meet expectations");
    CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
  }
}

TEST(memory_stack, edge_lower)
{
  libskiff::machine::memory::stack_c skiff_stack;
  {
    auto [okay, value] = skiff_stack.pop_word();
    CHECK_FALSE_TEXT(okay, "Able to pop word from empty stack");
  }
  {
    auto [okay, value] = skiff_stack.pop_dword();
    CHECK_FALSE_TEXT(okay, "Able to pop dword from empty stack");
  }
  {
    auto [okay, value] = skiff_stack.pop_qword();
    CHECK_FALSE_TEXT(okay, "Able to pop qword from empty stack");
  }

  CHECK_TRUE_TEXT(skiff_stack.push_word(0xFFFF), "Unable to push word");

  {
    auto [okay, value] = skiff_stack.pop_qword();
    CHECK_FALSE_TEXT(okay, "Able to pop qword from small stack");
  }
  {
    auto [okay, value] = skiff_stack.pop_dword();
    CHECK_FALSE_TEXT(okay, "Able to pop dword from small stack");
  }
  {
    auto [okay, value] = skiff_stack.pop_word();
    CHECK_TRUE_TEXT(okay, "Unable to pop value from stack");
    CHECK_EQUAL_TEXT(0xFFFF, value, "Unexpected value");
  }
}

TEST(memory_stack, mem_words)
{
  libskiff::types::vm_register stack_pointer = 0;
  libskiff::machine::memory::stack_c skiff_stack;
  skiff_stack.set_sp(stack_pointer);

  auto test_case = generate_tc<uint16_t>();
  uint64_t idx = 0;
  for (auto &tc : test_case) {
    CHECK_TRUE_TEXT(skiff_stack.store_word(idx, tc), "Unable to store word");
    idx += libskiff::system::word_size_bytes;
  }

  CHECK_EQUAL_TEXT(0, stack_pointer, "Stack pointer updated on non push-pop operation");

  idx = 0;
  for (auto &tc : test_case) {
    auto [okay, value] = skiff_stack.load_word(idx);
    CHECK_TRUE_TEXT(okay, "No success getting word");
    CHECK_EQUAL_TEXT(tc, value, "Value did not meet expectations");
    idx += libskiff::system::word_size_bytes;
  }
}

TEST(memory_stack, mem_dwords)
{
  libskiff::types::vm_register stack_pointer = 0;
  libskiff::machine::memory::stack_c skiff_stack;
  skiff_stack.set_sp(stack_pointer);

  auto test_case = generate_tc<uint32_t>();
  uint64_t idx = 0;
  for (auto &tc : test_case) {
    CHECK_TRUE_TEXT(skiff_stack.store_dword(idx, tc), "Unable to store dword");
    idx += libskiff::system::d_word_size_bytes;
  }

  CHECK_EQUAL_TEXT(0, stack_pointer, "Stack pointer updated on non push-pop operation");

  idx = 0;
  for (auto &tc : test_case) {
    auto [okay, value] = skiff_stack.load_dword(idx);
    CHECK_TRUE_TEXT(okay, "No success getting dword");
    CHECK_EQUAL_TEXT(tc, value, "Value did not meet expectations");
    idx += libskiff::system::d_word_size_bytes;
  }
}

TEST(memory_stack, mem_qwords)
{
  libskiff::types::vm_register stack_pointer = 0;
  libskiff::machine::memory::stack_c skiff_stack;
  skiff_stack.set_sp(stack_pointer);

  auto test_case = generate_tc<uint64_t>();
  uint64_t idx = 0;
  for (auto &tc : test_case) {
    CHECK_TRUE_TEXT(skiff_stack.store_qword(idx, tc), "Unable to store qword");
    idx += libskiff::system::q_word_size_bytes;
  }

  CHECK_EQUAL_TEXT(0, stack_pointer, "Stack pointer updated on non push-pop operation");

  idx = 0;
  for (auto &tc : test_case) {
    auto [okay, value] = skiff_stack.load_qword(idx);
    CHECK_TRUE_TEXT(okay, "No success getting qword");
    CHECK_EQUAL_TEXT(tc, value, "Value did not meet expectations");
    idx += libskiff::system::q_word_size_bytes;
  }
}

TEST(memory_stack, chonker)
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

  libskiff::types::vm_register stack_pointer = 0;
  libskiff::machine::memory::stack_c skiff_stack;
  skiff_stack.set_sp(stack_pointer);
  libskiff::types::vm_register expected_sp = 0;

  // Insert all cases
  for (auto &i : test_cases) {
    switch (i.type) {
    case type_t::WORD:
      CHECK_TRUE(skiff_stack.push_word(static_cast<uint16_t>(i.value)));
      expected_sp += libskiff::system::word_size_bytes;
      CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
      break;
    case type_t::DWORD:
      CHECK_TRUE(skiff_stack.push_dword(static_cast<uint32_t>(i.value)));
      expected_sp += libskiff::system::d_word_size_bytes;
      CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
      break;
    case type_t::QWORD:
      CHECK_TRUE(skiff_stack.push_qword(i.value));
      expected_sp += libskiff::system::q_word_size_bytes;
      CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
      break;
    default:
      FAIL("Random number gen broke the freakin tests");
      break;
    };
  }

  for (auto i = test_cases.rbegin(); i != test_cases.rend(); i++) {
    switch ((*i).type) {
    case type_t::WORD: {
      auto [okay, value] = skiff_stack.pop_word();
      CHECK_TRUE(okay);
      CHECK_EQUAL((*i).value, static_cast<uint64_t>(value));
      expected_sp -= libskiff::system::word_size_bytes;
      CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
    } break;
    case type_t::DWORD: {
      auto [okay, value] = skiff_stack.pop_dword();
      CHECK_TRUE(okay);
      CHECK_EQUAL((*i).value, static_cast<uint64_t>(value));
      expected_sp -= libskiff::system::d_word_size_bytes;
      CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
    } break;
    case type_t::QWORD: {
      auto [okay, value] = skiff_stack.pop_qword();
      CHECK_TRUE(okay);
      CHECK_EQUAL((*i).value, value);
      expected_sp -= libskiff::system::q_word_size_bytes;
      CHECK_EQUAL_TEXT(expected_sp, stack_pointer, "Stack pointer was not updated");
    } break;
    default:
      FAIL("Random number gen broke the freakin tests");
      break;
    }
  }
}
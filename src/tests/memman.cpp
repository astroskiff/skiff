#include <libskiff/machine/memory/memman.hpp>
#include <vector>

#include <CppUTest/TestHarness.h>

TEST_GROUP(memman_tests){};

TEST(memman_tests, all)
{
  // Create the thing
  skiff::machine::memory::memman_c memman;

  std::vector<uint64_t> ids;

  // Add 5 items
  for (auto i = 0; i < 5; i++) {
    auto [okay, id] = memman.alloc(1024);
    CHECK_TRUE_TEXT(okay, "Unable to alloc 1024 bytes");
    CHECK_EQUAL_TEXT(i, id, "Unexpected ID generated");
  }

  // Delete the middle one
  CHECK_TRUE_TEXT(memman.free(2), "Unable to free middle item");

  // Create a new one
  {
    auto [okay, id] = memman.alloc(1024);
    CHECK_TRUE_TEXT(okay, "Unable to re-alloc 1024 bytes");

    // Ensure new one has id of 2
    CHECK_EQUAL_TEXT(
        2, id, "Unexpected ID generated - ID queue implementation change?");
  }

  {
    auto [okay, id] = memman.alloc(1024);
    CHECK_TRUE_TEXT(okay, "Unable to alloc 1024 bytes");
    CHECK_EQUAL_TEXT(5, id, "Unexpected ID generated");
  }

  // Attempt to delete item 6 (failure expected)
  CHECK_FALSE_TEXT(memman.free(6), "Able to free non-existent item");
}

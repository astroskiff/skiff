#include <CppUTest/TestHarness.h>

#include <libskiff/coder.hpp>
#include <libutil/generate_random.hpp>

namespace {
  constexpr uint64_t num_tests = 100;
}

TEST_GROUP(coder_tests){};

TEST(coder_tests, endecode16)
{
  for (auto i = 0; i < num_tests; i++) {
    uint16_t value = libutil::generate::generate_random_c<uint8_t>().get_range(
        0, std::numeric_limits<uint8_t>::max());
    auto encoded = libskiff::coder::encode_16(value);
    CHECK_EQUAL(value, libskiff::coder::decode_16(encoded));
  }
}


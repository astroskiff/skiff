#include "CppUTest/TestHarness.h"
#include <libskiff/floating_point.hpp>
#include <libutil/generate_random.hpp>
#include <limits>
#include <stdint.h>
#include <iostream>

TEST_GROUP(floating_point){void setup(){}
                           void teardown(){
                           }};

TEST(floating_point, construction_deconstruction)
{
  for (auto i = 0; i < 100; i++) {
    auto value = libutil::generate::generate_random_c<double>().get_range(
        std::numeric_limits<double>::min(), std::numeric_limits<double>::max());

    // Deconstruct it
    libskiff::floating_point::deconstructed_t d =
        libskiff::floating_point::deconstruct(value);

    auto out = libskiff::floating_point::construct(d);

    // Construct it and make sure its the same
    //
    CHECK_EQUAL(value, out);
  }
}

TEST(floating_point, equality)
{
  for (auto i = 0; i < 100; i++) {
    auto value = libutil::generate::generate_random_c<double>().get_range(
        std::numeric_limits<double>::min(), std::numeric_limits<double>::max());

    auto modified = value;
    CHECK_EQUAL(value, modified);
    CHECK_TRUE(libskiff::floating_point::are_equal(value, modified));
  }
}

TEST(floating_point, encode_decode)
{
  for (auto i = 0; i < 100; i++) {
    auto value = libutil::generate::generate_random_c<double>().get_range(
        std::numeric_limits<double>::min(), std::numeric_limits<double>::max());

    CHECK_EQUAL(value, libskiff::floating_point::from_uint64_t(
                           libskiff::floating_point::to_uint64_t(value)));
  }
}
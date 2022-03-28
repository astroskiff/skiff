
#include <set>
#include <string>
#include <libutil/random_string.hpp>

#include "CppUTest/TestHarness.h"
TEST_GROUP(RandomStrings){void setup(){}

                          void teardown(){

                          }};

TEST(RandomStrings, length)
{
  //  Generate random strings and ensure that the length is what we expect
  //

  libutil::generate::random_string_c random_string;

  for (auto i = 10; i < 20; i++) {
    auto value = random_string.generate_string(i);

    CHECK_TRUE_TEXT((value.length() == i),
                    "Generated string was not expected length");
  }
}

TEST(RandomStrings, values)
{
  //  Create a string to source random stuff from
  //
  std::string test_string = "abcdef";

  //  Figure out how many times we will call generate
  //
  auto iterations = test_string.size() * 100;

  //  Create the random string class with the source string
  //
  libutil::generate::random_string_c random_string(test_string);

  //  Generate strings
  //
  std::set<std::string> s;
  for (auto i = 0; i < iterations; i++) {
    // Get 1 random char
    //
    auto value = random_string.generate_string(1);

    //  Add it to the set
    //
    s.insert(value);
  }

  //  Just in case there is an error we make the message
  //
  std::string error_string =
      "Either a statistical anomaly has occurred or the random entry generator "
      "failed to generate all possible string datums (";
  error_string += std::to_string(test_string.size()) + " total types after " +
                  std::to_string(iterations) + " calls)";

  //  Iterate over the test string and ensure that every char that could have
  //  been generated was generated
  //
  for (auto item : test_string) {
    if (0 == s.count(std::string(1, item))) {
      FAIL(error_string.c_str());
    }
  }
}

#include <set>
#include <stdint.h>
#include <vector>

#include <libutil/random_entry.hpp>

#include "CppUTest/TestHarness.h"

namespace {
//  Number of items per enumeration to expect
//
constexpr size_t NUM_VALUES_PER_TC = 6;

//  Number of items to generate random values - This number can change with no
//  issue although if it is too low it will not give the test ample time to
//  generate all possible values required to pass test
//
constexpr size_t NUM_GENERATIONS = 250;

//  Each class needs to have the same number and names of elements to ensure we
//  can test them with templates
//
enum class Tc1 { VAL_0, VAL_1, VAL_2, VAL_3, VAL_4, VAL_5 };
enum class Tc2 { VAL_0 = 0, VAL_1, VAL_2, VAL_3, VAL_4, VAL_5 };
enum class Tc3 { VAL_0 = 10, VAL_1, VAL_2, VAL_3 = 30, VAL_4, VAL_5 };
enum class Tc4 {
  VAL_0 = 1,
  VAL_1 = 2,
  VAL_2 = 3,
  VAL_3 = 4,
  VAL_4 = 5,
  VAL_5 = 6
};

//  Create a vector of enumeration values
//
template <class T> std::vector<T> get_vec()
{
  return {T::VAL_0, T::VAL_1, T::VAL_2, T::VAL_3, T::VAL_4, T::VAL_5};
}

//  Ensure that a given value is valid
//
template <class T> bool value_is_valid(T value)
{
  switch (value) {
  case T::VAL_0:
  case T::VAL_1:
  case T::VAL_2:
  case T::VAL_3:
  case T::VAL_4:
  case T::VAL_5:
    return true;
  default:
    return false;
  }
}
} // namespace

TEST_GROUP(RandomEntry){void setup(){} void teardown(){}};

TEST(RandomEntry, Enumerations)
{
  // Check that the checking function is correct
  //
  CHECK_FALSE_TEXT(value_is_valid<Tc1>(static_cast<Tc1>(-99999)),
                   "Helper function not working");
  CHECK_FALSE_TEXT(value_is_valid<Tc2>(static_cast<Tc2>(-99999)),
                   "Helper function not working");
  CHECK_FALSE_TEXT(value_is_valid<Tc3>(static_cast<Tc3>(-99999)),
                   "Helper function not working");
  CHECK_FALSE_TEXT(value_is_valid<Tc4>(static_cast<Tc4>(-99999)),
                   "Helper function not working");

  //  Create the vectors
  //
  std::vector<Tc1> t1 = get_vec<Tc1>();
  std::vector<Tc2> t2 = get_vec<Tc2>();
  std::vector<Tc3> t3 = get_vec<Tc3>();
  std::vector<Tc4> t4 = get_vec<Tc4>();

  //  Make sure the vectors are what we expect by size
  //
  CHECK_TRUE_TEXT((NUM_VALUES_PER_TC == t1.size()),
                  "Failure detected in vector generation");
  CHECK_TRUE_TEXT((NUM_VALUES_PER_TC == t2.size()),
                  "Failure detected in vector generation");
  CHECK_TRUE_TEXT((NUM_VALUES_PER_TC == t3.size()),
                  "Failure detected in vector generation");
  CHECK_TRUE_TEXT((NUM_VALUES_PER_TC == t4.size()),
                  "Failure detected in vector generation");

  //  Make sure the vectors contain nothing but valid values
  //
  for (auto item : t1) {
    CHECK_TRUE_TEXT(value_is_valid<Tc1>(item),
                    "Generated vector contains invalid enumeration");
  }
  for (auto item : t2) {
    CHECK_TRUE_TEXT(value_is_valid<Tc2>(item),
                    "Generated vector contains invalid enumeration");
  }
  for (auto item : t3) {
    CHECK_TRUE_TEXT(value_is_valid<Tc3>(item),
                    "Generated vector contains invalid enumeration");
  }
  for (auto item : t4) {
    CHECK_TRUE_TEXT(value_is_valid<Tc4>(item),
                    "Generated vector contains invalid enumeration");
  }

  std::string error_string =
      "Either a statistical anomaly has occurred or the random entry generator "
      "failed to generate all possible enumerations (";
  error_string += std::to_string(NUM_VALUES_PER_TC) + " total types after " +
                  std::to_string(NUM_GENERATIONS) + " calls)";

  //  Now we can test the random generation stuff (have to be sure that the
  //  helper functions are solid)
  //
  {
    std::set<Tc1> s;
    for (auto i = 0; i < NUM_GENERATIONS; i++) {
      auto value = libutil::generate::random_entry_c<Tc1>(t1).get_value();
      CHECK_TRUE_TEXT(value_is_valid<Tc1>(value),
                      "Randomly selected item from vector is not valid");
      s.insert(value);
    }
    for (auto item : t1) {
      if (0 == s.count(item)) {
        FAIL(error_string.c_str());
      }
    }
  }
  {
    std::set<Tc2> s;
    for (auto i = 0; i < NUM_GENERATIONS; i++) {
      auto value = libutil::generate::random_entry_c<Tc2>(t2).get_value();
      CHECK_TRUE_TEXT(value_is_valid<Tc2>(value),
                      "Randomly selected item from vector is not valid");
      s.insert(value);
    }
    for (auto item : t2) {
      if (0 == s.count(item)) {
        FAIL(error_string.c_str());
      }
    }
  }
  {
    std::set<Tc3> s;
    for (auto i = 0; i < NUM_GENERATIONS; i++) {
      auto value = libutil::generate::random_entry_c<Tc3>(t3).get_value();
      CHECK_TRUE_TEXT(value_is_valid<Tc3>(value),
                      "Randomly selected item from vector is not valid");
      s.insert(value);
    }
    for (auto item : t3) {
      if (0 == s.count(item)) {
        FAIL(error_string.c_str());
      }
    }
  }
  {
    std::set<Tc4> s;
    for (auto i = 0; i < NUM_GENERATIONS; i++) {
      auto value = libutil::generate::random_entry_c<Tc4>(t4).get_value();
      CHECK_TRUE_TEXT(value_is_valid<Tc4>(value),
                      "Randomly selected item from vector is not valid");
      s.insert(value);
    }
    for (auto item : t4) {
      if (0 == s.count(item)) {
        FAIL(error_string.c_str());
      }
    }
  }
}

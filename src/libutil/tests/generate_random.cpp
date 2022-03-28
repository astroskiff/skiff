#include <stdint.h>
#include <libutil/generate_random.hpp>
#include "CppUTest/TestHarness.h"

TEST_GROUP(GenerateRandom)
{
    void setup()
    {
    }

    void teardown()
    {

    }
};

TEST(GenerateRandom, iterative)
{
    // With the current implementation of the GenerateRandom we are essentially testing the std library, but these
    // tests ensure that updates to the mechanics of the class don't cause too much ruckus

    for(auto i = 0; i < 100; i++)
    {
        auto value = libutil::generate::generate_random_c<uint8_t>().get_range(0, 10);
        CHECK_TRUE_TEXT(
                (value >= 0 && value <= 10),
                "Value was out side of specified range"
                );
    }
    for(auto i = 0; i < 100; i++)
    {
        auto value = libutil::generate::generate_random_c<int8_t>().get_range(-10, 0);
        CHECK_TRUE_TEXT(
                (value >= -10 && value <= 10),
                "Value was out side of specified range"
        );
    }
    for(auto i = 0; i < 100; i++)
    {
        auto value = libutil::generate::generate_random_c<int8_t>().get_range(-10, 10);
        CHECK_TRUE_TEXT(
                (value >= -10 && value <= 10),
                "Value was out side of specified range"
        );
    }
}

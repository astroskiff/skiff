#ifndef SKIFF_SYSTEM_HPP
#define SKIFF_SYSTEM_HPP

#include <cstdint>

namespace skiff {
namespace config {
// These constants can be configured without issue
static constexpr uint64_t stack_size_bytes = 1'048'576;

// These constants should not be changed
static constexpr uint8_t word_size_bytes = 2;
static constexpr uint8_t d_word_size_bytes = word_size_bytes * 2;
static constexpr uint8_t q_word_size_bytes = d_word_size_bytes * 2;
static constexpr uint8_t num_integer_registers = 10;
static constexpr uint8_t num_floating_point_registers = 10;

} // namespace config
} // namespace skiff

#endif
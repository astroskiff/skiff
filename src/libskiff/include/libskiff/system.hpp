#ifndef LIBSKIFF_SYSTEM_HPP
#define LIBSKIFF_SYSTEM_HPP

namespace libskiff {
namespace system {
// These constants can be configured without issue
static constexpr uint64_t stack_size_bytes = 1'048'576;

// These constants should not be changed
static constexpr uint8_t word_size_bytes = 2;
static constexpr uint8_t d_word_size_bytes = word_size_bytes * 2;
static constexpr uint8_t q_word_size_bytes = d_word_size_bytes * 2;
} // namespace system
} // namespace libskiff

#endif
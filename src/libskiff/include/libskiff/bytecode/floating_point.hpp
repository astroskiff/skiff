#ifndef LIBSKIFF_FLOATING_POINT_HPP
#define LIBSKIFF_FLOATING_POINT_HPP

#include <cstdint>

namespace libskiff {
namespace bytecode {
namespace floating_point {

//! \brief Components of a floating point data type
struct deconstructed_t {
  bool sign;
  uint64_t exponent;
  uint64_t mantissa;
};

//! \brief Deconstruct a double into its base components
//! \param val The double to deconstruct
//! \returns Struct containing double components
extern deconstructed_t deconstruct(const double value);

//! \brief Construct a double from deconstructed components
//! \param dd The deconstructed double
//! \returns Constructed double
extern double construct(const deconstructed_t deconstructed);

//! \brief Convert a double to a uint64_t
//! \param value The value to convert to a uint64_t
//! \returns A uint64_t that was constructed from value
extern uint64_t to_uint64_t(const double value);

//! \brief Convert a uint64_t to a double
//! \param value The value to convert to a double
//! \returns A double that was constructed from value
extern double from_uint64_t(const uint64_t value);

//! \brief Check if two doubles are mostly equal
//! \param lhs Left hand operand
//! \param rhs Right hand operand
//! \returns true iff lhs and rhs are equal within given precision
//! \note Uses a precision of 0.00001 as default
extern bool are_equal(const double lhs, const double rhs,
                      const double precision = 0.00001);

} // namespace floating_point
} // namespace bytecode
} // namespace libskiff
#endif

#include "libskiff/floating_point.hpp"
#include <cmath>

namespace libskiff {
namespace floating_point {

namespace {

constexpr uint64_t IEEE_SIGN_MASK = 0x8000000000000000;
constexpr uint16_t IEEE_EXPONENT_MASK = 0x7FF;
constexpr unsigned IEEE_EXPONENT_SHIFT = 52;
constexpr uint64_t IEEE_MANTISSA_MASK = 0x000FFFFFFFFFFFFF;

// Union for easy conversions between doubles and uint64_ts
union extract_u {
  uint64_t bin;
  double d;
};
} // namespace

deconstructed_t deconstruct(const double value)
{
  deconstructed_t deconstructed;

  extract_u de;
  de.d = value;

  return deconstructed_t{std::signbit(value),
                         ((de.bin >> IEEE_EXPONENT_SHIFT) & IEEE_EXPONENT_MASK),
                         de.bin & IEEE_MANTISSA_MASK};
}

double construct(const struct deconstructed_t dd)
{
  if (dd.exponent & ~IEEE_EXPONENT_MASK) {
    return 0.0;
  }

  if (dd.mantissa & ~IEEE_MANTISSA_MASK) {
    return 0.0;
  }

  extract_u de;

  de.bin = (dd.sign ? IEEE_SIGN_MASK : 0x00) |
           static_cast<uint64_t>((dd.exponent & IEEE_EXPONENT_MASK))
               << IEEE_EXPONENT_SHIFT |
           static_cast<uint64_t>((dd.mantissa & IEEE_MANTISSA_MASK));

  double d = de.d;
  return d;
}

bool are_equal(const double lhs, const double rhs, double precision)
{
  return std::fabs(lhs - rhs) < precision;
}

uint64_t to_uint64_t(const double value)
{
  extract_u d;
  d.d = value;
  return d.bin;
}

double from_uint64_t(const uint64_t value)
{
  extract_u d;
  d.bin = value;
  return d.d;
}

} // namespace floating_point
} // namespace libskiff

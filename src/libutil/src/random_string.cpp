#include "libutil/random_string.hpp"
#include "libutil/generate_random.hpp"

#include <utility>

namespace libutil {
namespace generate {

random_string_c::random_string_c() : _source_string(random_string_c::ALL_CHARS)
{
}

random_string_c::random_string_c(const std::string &source_string)
    : _source_string(source_string)
{
}

std::string random_string_c::generate_string(size_t length) const
{
  if (length == 0)
    return {};

  std::string result;
  for (auto i = 0; i < length; i++) {
    auto next_item =
        generate_random_c<size_t>().get_range(0, _source_string.size() - 1);
    result += _source_string.at(next_item);
  }

  return result;
}
} // namespace generate
} // namespace libutil

#ifndef LIBSKIFF_VERSION_HPP
#define LIBSKIFF_VERSION_HPP

#include "libskiff/types.hpp"

namespace libskiff
{
namespace version
{
  static constexpr libskiff::types::semver_t semantic_version = {
    .major = 0,
    .minor = 0,
    .patch = 0
  };
}
}

#endif
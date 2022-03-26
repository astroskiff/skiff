#ifndef LIBUTIL_RANDOM_ENTRY_HPP
#define LIBUTIL_RANDOM_ENTRY_HPP

#include "generate_random.hpp"
#include <vector>

namespace libutil {
namespace generate {
//! \brief Generate a random value from a set of values
//! \tparam T The type of data that will exist within the list of possible
//! entries
template <class T> class random_entry_c {
public:
  //! \brief Create the object that can generate a random entry
  //! \param values The list of possible values to pull random entries from
  explicit random_entry_c(std::vector<T> values) : _values(values) {}

  //! \brief Get a value from the range of given values
  //! \returns Random entry of type T from given list
  T get_value()
  {
    generate_random_c<int64_t> r;
    auto idx = r.get_range(0, _values.size() - 1);
    return _values[idx];
  }

private:
  std::vector<T> _values;
};
} // namespace generate
} // namespace libutil

#endif

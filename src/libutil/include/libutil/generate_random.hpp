#ifndef LIBUTIL_GENERATE_RANDOM_HPP
#define LIBUTIL_GENERATE_RANDOM_HPP

#include <random>

namespace libutil {
namespace generate {

//! \brief  Simple generator for random numerical types used in testing
//! \tparam T Given numerical type that the generator should supply upon the
//! call to get_range
template <typename T> class generate_random_c {
public:
  //! \brief Setup the object
  explicit generate_random_c() : eng(rd()) {}

  //! \brief     Get a numerical value within a specified range
  //! \param min Minimum value
  //! \param max Maximum Value
  //! \return    Return the randomly generated type T value
  T get_range(T min, T max)
  {
    std::uniform_int_distribution<T> dist(min, max);
    return dist(eng);
  }

  //! \brief     Get a floating point value within a specified range
  //! \param min Minimum value
  //! \param max Maximum Value
  //! \return    Return the randomly generated type T value
  T get_floating_point_range(T min, T max)
  {
    std::uniform_real_distribution<T> dist(min, max);
    return dist(eng);
  }

private:
  std::random_device rd;
  std::default_random_engine eng;
};
} // namespace generate
} // namespace libutil

#endif

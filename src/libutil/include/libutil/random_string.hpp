#ifndef LIBUTIL_RANDOM_STRING_HPP
#define LIBUTIL_RANDOM_STRING_HPP

#include <string>

namespace libutil {
namespace generate {

//! \brief Generate random strings
class random_string_c {
public:
  //! A set of chars that can be used as the random source pool
  static constexpr char ALL_CHARS[] = "0123456789"
                                      "~!@#$%^&*()_+"
                                      "`-=<>,./?\"';:"
                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                      "abcdefghijklmnopqrstuvwxyz";

  //! A set of alphanumeric chars that can be used as the random source pool
  static constexpr char ALPHA_NUM[] = "0123456789"
                                      "~!@#$%^&*()_+"
                                      "`-=<>,./?\"';:"
                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                      "abcdefghijklmnopqrstuvwxyz";

  //! \brief Create the object with the default source pool (ALL_CHARS)
  //! \post  All calls to generate_string will pull from chars in ALL_CHARS
  random_string_c();

  //! \brief Create the object with the a specified source string
  //! \param source_string The string data to source random chars from
  //! \post  All calls to generate_string will pull from chars in source_string
  random_string_c(const std::string &source_string);

  //! \brief   Generate a random string
  //! \param   length Generate a random sting of the given length
  //! \returns String of a random value containing only chars from the given
  //! source
  std::string generate_string(size_t length) const;

private:
  std::string _source_string;
};
} // namespace generate
} // namespace libutil

#endif

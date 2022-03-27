// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__DIGITSET_HPP_
#define CBR_UTILS__DIGITSET_HPP_

#include <array>
#include <bitset>
#include <utility>

namespace cbr {

namespace detail {

template<std::size_t e = 2>
constexpr std::size_t pow_fast(const std::size_t b) noexcept
{
  if constexpr (e == 0) { return 1; }

  if constexpr (e == 1) { return b; }

  std::size_t out = b;
  for (std::size_t i = 2; i <= e; i++) { out *= b; }
  return out;
}
}  // namespace detail

/**
 * @brief Extention of std::bitset for bases > 2.
 * @details Allows to easily extract the digits of a number in a given base.
 *
 * @tparam N_DIGITS Maximum number of digits to use for representation.
 * @tparam BASE Base used to represent the number (default is 10).
 */
template<std::size_t N_DIGITS, std::size_t BASE = 10>
class digitset
{
  static_assert(N_DIGITS > 0, "Length must be > 0");
  static_assert(BASE > 0, "Base must be > 0");

public:
  digitset()                 = default;
  digitset(const digitset &) = default;
  digitset(digitset &&)      = default;
  digitset & operator=(const digitset &) = default;
  digitset & operator=(digitset &&) = default;
  ~digitset()                       = default;

  /**
   * @brief Construct digit set from a value.
   * @details It is the value modulo BASE^N_DIGITS that is being used.
   *
   * @param
   */
  constexpr digitset(std::size_t val) noexcept
  {
    val %= detail::pow_fast<N_DIGITS>(BASE);

    std::size_t i = 0;
    while (val > 0 && i < N_DIGITS) {
      m_data[i++] = val % BASE;
      val /= BASE;
    }
    for (; i < N_DIGITS; ++i) { m_data[i] = 0; }
  }

  /**
   * @brief Get the nth digit of the represented number.
   *
   * @return The nth digit of the represented number.
   */
  constexpr std::size_t operator[](const std::size_t n) const { return m_data[n]; }

  /**
   * @brief Get the nth digit of the represented number.
   *
   * @return A reference to the nth digit of the represented number.
   */
  std::size_t & operator[](const std::size_t n) { return m_data[n]; }

  /**
   * @brief Get the size of the digit set.
   * @details Returns N_DIGITS
   *
   * @return The size of the digit set.
   */
  constexpr std::size_t size() const noexcept { return N_DIGITS; }

  /**
   * @brief Get the nth digit of the represented number.
   * @details Performs bound checking.
   *
   * @return The nth digit of the represented number.
   */
  std::size_t test(const std::size_t n) const { return m_data.at(n); }

  /**
   * @brief Get the number of non-zero digits of the represented number.
   *
   * @return The number of non-zero digits of the represented number.
   */
  constexpr std::size_t count() const noexcept
  {
    std::size_t out = 0;

    for (const auto & d : m_data) {
      if (d != 0) { ++out; }
    }

    return out;
  }

  /**
   * @brief Get the represented number.
   *
   * @return The represented number as an std::size_t;
   */
  constexpr std::size_t to_ulong() const noexcept
  {
    if constexpr (N_DIGITS == 1) { return m_data[0]; }

    std::size_t b   = BASE;
    std::size_t out = static_cast<std::size_t>(m_data[0]) + b * static_cast<std::size_t>(m_data[1]);
    if (N_DIGITS == 2) { return out; }

    std::size_t p = b;
    for (std::size_t i = 2; i < N_DIGITS; ++i) {
      p *= b;
      out += m_data[i] * p;
    }
    return out;
  }

private:
  std::array<std::size_t, N_DIGITS> m_data{};
};

/**
 * @brief Compute the set of n-tuples of all combinaisons given m possible values for each
 * element of the n-tuple.
 * @details There are m^n such tuples.
 *
 * Example:
 *
 * constexpr std::size_t n = 3;
 * constexpr std::size_t m = 2;
 * std::array<std::array<int, m>, n> vals{{{1, 2}, {3, 4}, {5, 6}}};
 * const auto perms = digit_perm(vals);
 *
 * Then perms == {{1,3,5}, {1,3,6}, {1,4,5}, {1,4,6}, {2,3,5}, {2,3,6}, {2,4,5}, {2,4,6}};
 *
 * @tparam n Size of the tuple.
 * @tparam m Number of possible values per element of the tuple.
 * @tparam T Type of elements of the tuple.
 *
 * @param vals n-arrays of all m-arrays of possible values for each element of the n-tuple.
 * @return m^n-arrays of all combinaisons of n-arrays.
 */
template<std::size_t n, std::size_t m, typename T>
constexpr auto digit_perm(const std::array<std::array<T, m>, n> & vals) noexcept
{
  const std::size_t N = detail::pow_fast<n>(m);
  std::array<std::array<T, n>, N> out{};

  for (std::size_t i = 0; i < N; i++) {
    const digitset<n, m> currComb(i);
    for (std::size_t j = 0; j < n; j++) { out[i][j] = vals[j][currComb[j]]; }
  }

  return out;
}

/**
 * @brief Compute the set of n-tuples of all combinaisons given m possible values for each
 * element of the n-tuple.
 * @details There are m^n such tuples. The possible values are the same for each element of the
 * tuple.
 *
 * Example:
 *
 * constexpr std::size_t n = 3;
 * constexpr std::size_t m = 2;
 * std::array<int, m> vals{{1, 2}};
 * const auto perms = digit_perm<n>(vals);
 *
 * Then perms == {{1,1,1}, {1,1,2}, {1,2,1}, {1,2,2}, {2,1,1}, {2,1,2}, {2,2,1}, {2,2,2}};
 *
 * @tparam n Size of the tuple.
 * @tparam m Number of possible values per element of the tuple.
 * @tparam T Type of elements of the tuple.
 *
 * @param vals m-arrays of possible values for each element of the n-tuple.
 * @return m^n-arrays of all combinaisons of n-arrays.
 */
template<std::size_t n, std::size_t m, typename T>
constexpr auto digit_perm(const std::array<T, m> & vals) noexcept
{
  const std::size_t N = detail::pow_fast<n>(m);
  std::array<std::array<T, n>, N> out{};

  for (std::size_t i = 0; i < N; i++) {
    const digitset<n, m> currComb(i);
    for (std::size_t j = 0; j < n; j++) { out[i][j] = vals[currComb[j]]; }
  }

  return out;
}

/**
 * @brief Compute the set of n-tuples of all combinaisons given m possible values for each
 * element of the n-tuple.
 * @details There are m^n such tuples. The possible values for each element of the tuple are
 * {0,...,m-1}.
 *
 * Example:
 *
 * constexpr std::size_t n = 3;
 * constexpr std::size_t m = 2;
 * const auto perms = digit_perm<n,m>();
 *
 * Then perms == {{0,0,0}, {0,0,1}, {0,1,0}, {0,1,1}, {1,0,0}, {1,0,1}, {1,1,0}, {1,1,1}};
 *
 * @tparam n Size of the tuple.
 * @tparam m Number of possible values per element of the tuple.
 * @tparam T Type of elements of the tuple.
 *
 * @param vals m-arrays of possible values for each element of the n-tuple.
 * @return m^n-arrays of all combinaisons of n-arrays.
 */
template<std::size_t n, std::size_t m, typename T = std::size_t>
constexpr auto digit_perm() noexcept
{
  const std::size_t N = detail::pow_fast<n>(m);
  std::array<std::array<T, n>, N> out{};

  for (std::size_t i = 0; i < N; i++) {
    const digitset<n, m> currComb(i);
    for (std::size_t j = 0; j < n; j++) { out[i][j] = static_cast<T>(currComb[j]); }
  }

  return out;
}

/**
 * @brief Compute the set of n-tuples of all combinaisons given 2 possible values for each
 * element of the n-tuple.
 * @details There are 2^n such tuples.
 *
 * Example:
 *
 * constexpr std::size_t n = 3;
 * std::array<int, n> min{{0,1,2}};
 * std::array<int, n> max{{3,4,5}};
 * const auto perms = binary_perm(min, max);
 *
 * Then perms == {{0,1,2}, {0,1,5}, {0,4,2}, {0,4,5}, {3,1,2}, {3,1,5}, {3,4,2}, {3,4,5}};
 *
 * @tparam n Size of the tuple.
 * @tparam T Type of elements of the tuple.
 *
 * @param min 1st n-array of possible values for each element of the n-tuple.
 * @param max 2nd n-array of possible values for each element of the n-tuple.
 * @return 2^n-arrays of all combinaisons of n-arrays.
 */
template<std::size_t n, typename T>
constexpr auto binary_perm(const std::array<T, n> & min, const std::array<T, n> & max) noexcept
{
  const std::size_t N = std::size_t(1) << n;
  std::array<std::array<T, n>, N> out{};

  for (std::size_t i = 0; i < N; i++) {
    const std::bitset<n> currComb(i);
    for (std::size_t j = 0; j < n; j++) {
      if (currComb[j]) {
        out[i][j] = max[j];
      } else {
        out[i][j] = min[j];
      }
    }
  }

  return out;
}

/**
 * @brief Compute the set of n-tuples of all combinaisons given 2 possible values for each
 * element of the n-tuple.
 * @details There are 2^n such tuples. The possible values are the same for each element of the
 * tuple.
 *
 * Example:
 *
 * constexpr std::size_t n = 3;
 * int min = 1;
 * int max = 5;
 * const auto perms = binary_perm<n>(min, max);
 *
 * Then perms == {{1,1,1}, {1,1,5}, {1,5,1}, {1,5,5}, {5,1,1}, {5,1,5}, {5,5,1}, {5,5,5}};
 *
 * @tparam n Size of the tuple.
 * @tparam T Type of elements of the tuple.
 *
 * @param min 1st possible value for each element of the n-tuple.
 * @param max 2nd possible value for each element of the n-tuple.
 * @return 2^n-arrays of all combinaisons of n-arrays.
 */
template<std::size_t n, typename T>
constexpr auto binary_perm(const T & min, const T & max) noexcept
{
  const std::size_t N = std::size_t(1) << n;
  std::array<std::array<T, n>, N> out{};

  for (std::size_t i = 0; i < N; i++) {
    const std::bitset<n> currComb(i);
    for (std::size_t j = 0; j < n; j++) {
      if (currComb[j]) {
        out[i][j] = max;
      } else {
        out[i][j] = min;
      }
    }
  }

  return out;
}

/**
 * @brief Compute the set of n-tuples of all combinaisons given 2 possible values for each
 * element of the n-tuple.
 * @details There are 2^n such tuples. The possible values for each element of the tuple are 0 or 1.
 *
 * Example:
 *
 * constexpr std::size_t n = 3;
 * const auto perms = binary_perm<n>();
 *
 * Then perms == {{0,0,0}, {0,0,1}, {0,1,0}, {0,1,1}, {1,0,0}, {1,0,1}, {1,1,0}, {1,1,1}};
 *
 * @tparam n Size of the tuple.
 * @tparam T Type of elements of the tuple.
 *
 * @return 2^n-arrays of all combinaisons of n-arrays.
 */
template<size_t n, typename T = bool>
constexpr auto binary_perm() noexcept
{
  const std::size_t N = std::size_t(1) << n;
  std::array<std::array<T, n>, N> out{};

  for (std::size_t i = 0; i < N; i++) {
    const std::bitset<n> currComb(i);
    for (std::size_t j = 0; j < n; j++) { out[i][j] = currComb[j]; }
  }

  return out;
}

}  // namespace cbr

#endif  // CBR_UTILS__DIGITSET_HPP_

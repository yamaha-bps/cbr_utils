// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__DIGITSET_HPP_
#define CBR_UTILS__DIGITSET_HPP_

#include <array>
#include <bitset>
#include <utility>
#include <utility>

namespace cbr
{

namespace detail
{
template<std::size_t e = 2>
constexpr std::size_t pow_fast(const std::size_t b) noexcept
{
  if constexpr (e == 0) {
    return 1;
  }

  if constexpr (e == 1) {
    return b;
  }

  std::size_t out = b;
  for (std::size_t i = 2; i <= e; i++) {
    out *= b;
  }
  return out;
}
}

/**
 * @brief extention of std::bitset for bases > 2,
 * i.e. extract the digits of a number in a given base
 */
template<std::size_t _len, std::size_t _base = 10>
class digitset
{
  static_assert(_base > 0, "Base must be > 0");
  static_assert(_len > 0, "Length must be > 0");

public:
  digitset() = default;
  digitset(const digitset &) = default;
  digitset(digitset &&) = default;
  digitset & operator=(const digitset &) = default;
  digitset & operator=(digitset &&) = default;
  ~digitset() = default;

/**
 * @brief construct digit set from value
 * If input has more digits than the specified length of the digit set,
 * then it is the input modulus base^lenth that is used.
 */
  constexpr digitset(std::size_t val) noexcept
  {
    val %= detail::pow_fast<_len>(_base);

    std::size_t i = 0;
    while (val > 0 && i < _len) {
      data_[i++] = val % _base;
      val /= _base;
    }
  }

  /**
   * @brief access the digit at a given position.
   */
  constexpr std::size_t operator[](const std::size_t pos) const
  {
    return data_[pos];
  }

  /**
   * @brief access the digit at a given position.
   */
  std::size_t & operator[](const std::size_t pos)
  {
    return data_[pos];
  }

  /**
   * @brief returns the size of the digit set
   */
  constexpr std::size_t size() const noexcept
  {
    return _len;
  }

  /**
   * @brief access the digit at a given position with bound checking.
   */
  std::size_t test(const std::size_t pos) const
  {
    return data_.at(pos);
  }

  /**
   * @brief returns the number of non-zero digits
   */
  constexpr std::size_t count() const noexcept
  {
    std::size_t out = 0;

    for (const auto & d : data_) {
      if (d != 0) {
        out++;
      }
    }
    return out;
  }

  /**
   * @brief converts the digit set back to an std::size_t
   */
  constexpr std::size_t to_ulong() const noexcept
  {
    if (_len == 1) {
      return data_[0];
    }

    std::size_t b = _base;
    std::size_t out = static_cast<std::size_t>(data_[0]) +
      b * static_cast<std::size_t>(data_[1]);
    if (_len == 2) {
      return out;
    }

    std::size_t p = b;
    for (std::size_t i = 2; i < _len; i++) {
      p *= b;
      out += data_[i] * p;
    }
    return out;
  }

private:
  std::array<std::size_t, _len> data_{};
};

/***************************************************************************
 * \brief Compute the set of arrays of all possible one-to-one permutations
 *  of elements between the N inputs
 ***************************************************************************/
template<std::size_t n, std::size_t b, typename T>
constexpr auto digit_perm(const std::array<std::array<T, b>, n> & vals) noexcept
{
  const std::size_t N = detail::pow_fast<n>(b);
  std::array<std::array<T, n>, N> out{};

  for (std::size_t i = 0; i < N; i++) {
    const digitset<n, b> currComb(i);
    for (std::size_t j = 0; j < n; j++) {
      out[i][j] = vals[j][currComb[j]];
    }
  }

  return out;
}

template<std::size_t n, std::size_t b, typename T>
constexpr auto digit_perm(const std::array<T, b> & vals) noexcept
{
  const std::size_t N = detail::pow_fast<n>(b);
  std::array<std::array<T, n>, N> out{};

  for (std::size_t i = 0; i < N; i++) {
    const digitset<n, b> currComb(i);
    for (std::size_t j = 0; j < n; j++) {
      out[i][j] = vals[currComb[j]];
    }
  }

  return out;
}

template<std::size_t n, std::size_t b, typename T = std::size_t>
constexpr auto digit_perm() noexcept
{
  const std::size_t N = detail::pow_fast<n>(b);
  std::array<std::array<T, n>, N> out{};

  for (std::size_t i = 0; i < N; i++) {
    const digitset<n, b> currComb(i);
    for (std::size_t j = 0; j < n; j++) {
      out[i][j] = static_cast<T>(currComb[j]);
    }
  }

  return out;
}

/***************************************************************************
 * \brief Compute the set of arrays of all possible one-to-one permutations
 *  of elements between the 2 inputs
 ***************************************************************************/
template<std::size_t n, typename T>
constexpr auto binary_perm(
  const std::array<T, n> & min,
  const std::array<T, n> & max) noexcept
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

template<std::size_t n, typename T>
constexpr auto binary_perm(
  const T & min,
  const T & max) noexcept
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

template<size_t n, typename T = bool>
constexpr auto binary_perm() noexcept
{
  const std::size_t N = std::size_t(1) << n;
  std::array<std::array<T, n>, N> out{};

  for (std::size_t i = 0; i < N; i++) {
    const std::bitset<n> currComb(i);
    for (std::size_t j = 0; j < n; j++) {
      out[i][j] = currComb[j];
    }
  }

  return out;
}

}  // namespace cbr

#endif  // CBR_UTILS__DIGITSET_HPP_

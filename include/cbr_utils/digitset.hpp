// Copyright 2020 Yamaha Motor Corporation, USA
#ifndef CBR_UTILS__DIGITSET_HPP_
#define CBR_UTILS__DIGITSET_HPP_

#include <cstdint>
#include <array>
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

template<std::size_t _len, uint8_t _base = 10>
class digitset
{
  static_assert(_base > 1, "Base must be > 1.");
  static_assert(_len > 0, "Length must be > 0.");

public:
  digitset() = default;
  digitset(const digitset &) = default;
  digitset(digitset &&) = default;
  digitset & operator=(const digitset &) = default;
  digitset & operator=(digitset &&) = default;
  ~digitset() = default;

  constexpr digitset(std::size_t val) noexcept
  {
    val %= detail::pow_fast<_len>(_base);

    std::size_t i = 0;
    while (val > 0 && i < _len) {
      data_[i++] = val % _base;
      val /= _base;
    }
  }

  constexpr uint8_t operator[](const std::size_t pos) const
  {
    return data_[pos];
  }

  uint8_t & operator[](const std::size_t pos)
  {
    return data_[pos];
  }

  constexpr std::size_t size() const noexcept
  {
    return _len;
  }


  uint8_t test(std::size_t pos) const
  {
    return data_.at(pos);
  }

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

  constexpr std::size_t to_ulong() const noexcept
  {
    if (_len == 1) {
      return data_[0];
    }

    std::size_t b = static_cast<std::size_t>(_base);
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
  std::array<uint8_t, _len> data_{};
};

}  // namespace cbr

#endif  // CBR_UTILS__DIGITSET_HPP_

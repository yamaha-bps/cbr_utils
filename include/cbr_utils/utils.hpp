// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__UTILS_HPP_
#define CBR_UTILS__UTILS_HPP_

#include <array>
#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "digitset.hpp"
#include "integer_pack.hpp"
#include "integer_sequence.hpp"
#include "static_for.hpp"
#include "type_pack.hpp"

namespace cbr {

/***************************************************************************
 * \brief Case insensitive string comparaison
 ***************************************************************************/
inline bool strcmpi(const std::string_view str1, const std::string_view str2) noexcept
{
  return (str1.size() == str2.size())
      && std::equal(
        str1.begin(), str1.end(), str2.begin(), [](const unsigned char c1, const unsigned char c2) {
          if (c1 == c2 || std::toupper(c1) == std::toupper(c2)) { return true; }
          return false;
        });
}

/***************************************************************************
 * \brief Convert timepoint to formated string
 ***************************************************************************/
template<typename clock_t = std::chrono::system_clock>
inline std::string dateStr(
  const typename clock_t::time_point & currentTime, const bool fullPrecision = false)
{
  time_t time = clock_t::to_time_t(currentTime);
  std::string nanoSecStr;

  if (fullPrecision) {
    auto currentTimeRounded = clock_t::from_time_t(time);
    if (currentTimeRounded > currentTime) {
      --time;
      currentTimeRounded -= std::chrono::seconds(1);
    }

    const auto nanosec =
      std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - currentTimeRounded)
        .count();

    nanoSecStr = "." + std::to_string(nanosec);
  }

  std::array<char, 80> timeBuffer{};
  strftime(timeBuffer.data(), timeBuffer.size(), "%Y-%m-%d_%H-%M-%S", std::localtime(&time));

  return std::string(timeBuffer.data()) + nanoSecStr;
}
template<typename clock_t = std::chrono::system_clock>
inline std::string dateStr(const bool fullPrecision = false)
{
  return dateStr(clock_t::now(), fullPrecision);
}

/***************************************************************************
 * \brief Convert a formated string to a timepoint
 ***************************************************************************/
template<typename clock_t = std::chrono::system_clock>
inline typename clock_t::time_point fromDateStr(const std::string & str)
{
  std::istringstream iss{str};
  std::tm tm{};

  if (!(iss >> std::get_time(&tm, "%Y-%m-%d_%H-%M-%S"))) {
    throw std::invalid_argument("Date has wrong format");
  }

  tm.tm_isdst = -1;

  const auto tOut = clock_t::from_time_t(std::mktime(&tm));
  const auto pos  = str.find('.');
  std::chrono::nanoseconds nanoSec(0);
  if (pos != std::string::npos) {
    try {
      nanoSec = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(std::stod(str.substr(pos))));
    } catch (const std::exception &) {}
  }

  return tOut + nanoSec;
}

/***************************************************************************
 * \brief Check if string is a valid filename for linux and windows
 ***************************************************************************/
inline bool isValidFilename(const std::string_view str)
{
  return !str.empty() && (str.find_first_of("\\/:*?\"<>|") == std::string::npos);
}

/***************************************************************************
 * \brief Takes a duration in second (as a double) and rounds to a duration
 * in the best units (s, ms, us)
 ***************************************************************************/
inline std::pair<double, std::string> formatDuration(const double t)
{
  double dt          = t;
  std::string dtUnit = "s";
  if (dt < 1.) {
    if (dt < 1e-3) {
      dt *= 1e6;
      dtUnit = "us";
    } else {
      dt *= 1e3;
      dtUnit = "ms";
    }
  }
  return {dt, dtUnit};
}

/***************************************************************************
 * \brief Check if all values in array are unique
 ***************************************************************************/
template<typename T, std::size_t N, typename BinaryPredicate>
constexpr bool all_unique(const std::array<T, N> & a, BinaryPredicate && p)
{
  for (std::size_t i = 0; i < N; i++) {
    for (std::size_t j = i + 1; j < N; j++) {
      if (p(a[i], a[j])) { return false; }
    }
  }
  return true;
}

template<typename T, std::size_t N>
constexpr bool all_unique(const std::array<T, N> & a)
{
  return all_unique<T, N, std::equal_to<T>>(a, std::equal_to<T>{});
}

/***************************************************************************
 * \brief Wrapper for lambda overload
 ***************************************************************************/
template<typename... Ts>
// cppcheck-suppress syntaxError
struct overload : Ts...
{
  using Ts::operator()...;
};

template<typename... Ts>
overload(Ts...) -> overload<Ts...>;

/***************************************************************************
 * \brief Extract signature of member function
 * Example
 *  using return_type = typename signature<decltype(&Class::Function)>::return_type
 *  using argument_type = typename signature<decltype(&Class::Function)>::argument_type
 ***************************************************************************/
template<typename S>
struct signature;

template<typename R, typename Cls, typename... Args>
struct signature<R (Cls::*)(Args...)>
{
  using return_type   = R;
  using argument_type = cbr::TypePack<Args...>;
};

template<typename R, typename Cls, typename... Args>
struct signature<R (Cls::*)(Args...) const>
{
  using return_type   = R;
  using argument_type = cbr::TypePack<Args...>;
};

/***************************************************************************
 * \brief Check if element in range are sorted
 ***************************************************************************/
namespace detail {
template<typename ForwardIt, template<typename> typename comp>
bool is_sorted_impl(ForwardIt first, ForwardIt last)
{
  using T = typename std::iterator_traits<ForwardIt>::value_type;

  if (std::distance(first, last) < 2) { return true; }

  const auto i = std::adjacent_find(first, last, comp<T>());

  if (i != last) { return false; }

  return true;
}

template<typename ExecutionPolicy, typename ForwardIt, template<typename> typename comp>
bool is_sorted_impl(ExecutionPolicy && policy, ForwardIt first, ForwardIt last)
{
  using T = typename std::iterator_traits<ForwardIt>::value_type;

  if (std::distance(first, last) < 2) { return true; }

  const auto i = std::adjacent_find(std::move(policy), first, last, comp<T>());

  if (i != last) { return false; }

  return true;
}
}  // namespace detail

template<typename ForwardIt>
bool is_sorted(ForwardIt first, ForwardIt last)
{
  return detail::is_sorted_impl<ForwardIt, std::greater>(first, last);
}

template<typename ExecutionPolicy, typename ForwardIt>
bool is_sorted(ExecutionPolicy && policy, ForwardIt first, ForwardIt last)
{
  return detail::is_sorted_impl<ForwardIt, std::greater>(std::move(policy), first, last);
}

template<class ForwardIt>
bool is_strictly_sorted(ForwardIt first, ForwardIt last)
{
  return detail::is_sorted_impl<ForwardIt, std::greater_equal>(first, last);
}

template<typename ExecutionPolicy, class ForwardIt>
bool is_strictly_sorted(ExecutionPolicy && policy, ForwardIt first, ForwardIt last)
{
  return detail::is_sorted_impl<ForwardIt, std::greater_equal>(std::move(policy), first, last);
}

}  // namespace cbr

#endif  // CBR_UTILS__UTILS_HPP_

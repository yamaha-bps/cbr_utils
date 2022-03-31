// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

/** @file */

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

/**
 * @brief Case insensitive string comparaison.
 * @details Input arguments are std::string_views.
 *
 * @param str1, str2 Strings to compare
 * @return Whether or not both strings are equal.
 */
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

/**
 * @brief Convert timepoint into string.
 * @details String format is %Y-%m-%d_%H-%M-%S(.nanoseconds).
 * See https://en.cppreference.com/w/cpp/chrono/c/strftime.
 * Example:
 * ```
 * const auto now = std::chrono::system_clock::now();
 * const auto s1 = dateStr(now);
 * const auto s2 = dateStr(now,true);
 * ```
 * Then s1 == "2022-01-01_15-13-54", and s2 == "2022-01-01_15-13-54.142542392".
 *
 * @tparam clock_t Type of the clock. Defines the type of the input arguments.
 * @param currentTime Timepoint to convert into a string.
 * @param fullPrecision If true, adds nanosecond value to output string (default: false).
 * @return std::string representation of the input timepoint.
 */
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

/**
 * @brief Overload of dateStr(clock_t currentTime, bool fullPrecision) with currentTime =
 * clock_t::now().
 */
template<typename clock_t = std::chrono::system_clock>
inline std::string dateStr(const bool fullPrecision = false)
{
  return dateStr(clock_t::now(), fullPrecision);
}

/***************************************************************************
 * \brief Convert a formated string to a timepoint
 ***************************************************************************/

/**
 * @brief Convert string generated by dateStr back to timepoint.
 * @details Works for strings generated with and without fullPrecision = true.
 *
 * @tparam clock_t Type of the clock. Defines the type of the returned value.
 * @param str std::string to be converted into a timepoint.
 * @return clock_t::time_point representation of the input string.
 */
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

/**
 * @brief Check if string is a valid Unix and Windows filename.
 * @details Can also only check for Unix validity.
 *
 * @param str String to check.
 * @param windows Check windows validity (default: true).
 * @return Boolean for whether or not string is a valid filename.
 */
inline bool isValidFilename(const std::string_view str, const bool windows = true)
{
  const bool linuxValid =
    !str.empty() && (str.find('/') == std::string::npos) && (str.find('\0') == std::string::npos);
  bool windowsValid = true;
  if (windows) {
    const char invalidBytes[31] = {1,
      2,
      3,
      4,
      5,
      6,
      7,
      8,
      9,
      10,
      11,
      12,
      13,
      14,
      15,
      16,
      17,
      18,
      19,
      20,
      21,
      22,
      23,
      24,
      25,
      26,
      27,
      28,
      29,
      30,
      31};

    windowsValid = (str.find_first_of("\\:*?\"<>|") == std::string::npos)
                && (str.find_first_of(invalidBytes) == std::string::npos) && !str.ends_with('.')
                && !str.ends_with(' ');
  }
  return linuxValid && windowsValid;
}

/**
 * @brief User friendly string representation of a duration.
 * @details Takes a duration in second (as a double) and rounds to a duration
 * in the most relevant units (min, s, ms, or us).
 *
 * @param t Duration (in seconds) to convert.
 * @return std::pair<double, std::string>, where the first member is the value of the duration
 * converted into the selected unit, and the second member is the acronym for the selected unit.
 */
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
  } else if (dt >= 60.) {
    dt /= 60.;
    dtUnit = "min";
  }

  return {dt, dtUnit};
}

/**
 * @brief Check if all values in std::array are unique.
 *
 * @tparam T Type of the elements of the array.
 * @tparam N Size of the array.
 * @tparam BinaryPredicate Type of the binary predicate to compare values of the array.
 * @param arr Array to check.
 * @param p Binary predicatate to compare values of the array (default: std::equal_to).
 * @return Whether or not all elements in the input array are different.
 */
template<typename T, std::size_t N, typename BinaryPredicate = std::equal_to<T>>
constexpr bool all_unique(const std::array<T, N> & arr, BinaryPredicate && p = {})
{
  for (std::size_t i = 0; i < N; i++) {
    for (std::size_t j = i + 1; j < N; j++) {
      if (p(arr[i], arr[j])) { return false; }
    }
  }
  return true;
}

/**
 * @brief Struct with overloaded call operators of all specified types.
 * @details Example:
 *
 * ```
 * const auto o = overload{
 *   [](int){std::cout << "int " << std::endl;},
 *   [](double){std::cout << "double " << std::endl;}
 * };
 * o(1);
 * o(1.);
 * ```
 * Then this would print: "int double ".
 * @tparam Ts Types of the callables to merge into the overload struct.
 */
template<typename... Ts>
// cppcheck-suppress syntaxError
struct overload : Ts...
{
  using Ts::operator()...;
};

/// @cond
// Custom template argument deduction
template<typename... Ts>
overload(Ts...) -> overload<Ts...>;
/// @endcond

/***************************************************************************
 * \brief
 *
 *
 *
 ***************************************************************************/

/**
 * @brief Extract signature of member function.
 * @details Example:
 * ```
 * struct S{void foo(double){}};
 *
 * using sig = signature<decltype(&S::foo)>;
 * using return_type = typename sig::return_type
 * using argument_type = typename sig::argument_type
 * ```
 * Then return_type == void and argument_type == double.
 *
 * @tparam S Type of the member function to extract signature out of.
 */
template<typename S>
struct signature;

/// @cond
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

/// @endcond

// is_sorted implementationm
/// @cond
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

  const auto i = std::adjacent_find(std::forward<ExecutionPolicy>(policy), first, last, comp<T>());

  if (i != last) { return false; }

  return true;
}
}  // namespace detail
/// @endcond

/**
 * @brief Check if range is sorted.
 * @details Uses std::adjacent_find behind the scenes:
 * https://en.cppreference.com/w/cpp/algorithm/adjacent_find.
 *
 * @param first, last The range of the elements to examine.
 * @return Boolean of whether or not the range is sorted.
 */
template<typename ForwardIt>
bool is_sorted(ForwardIt first, ForwardIt last)
{
  return detail::is_sorted_impl<ForwardIt, std::greater>(first, last);
}

/**
 * @brief Check if range is sorted.
 * @details Uses std::adjacent_find behind the scenes:
 * https://en.cppreference.com/w/cpp/algorithm/adjacent_find.
 *
 * @param policy Execution policy to use for std::adjacent_find:
 * https://en.cppreference.com/w/cpp/header/execution
 * @param first, last The range of the elements to examine.
 * @return Boolean of whether or not the range is sorted.
 */
template<typename ExecutionPolicy, typename ForwardIt>
bool is_sorted(ExecutionPolicy && policy, ForwardIt first, ForwardIt last)
{
  return detail::is_sorted_impl<ForwardIt, std::greater>(
    std::forward<ExecutionPolicy>(policy), first, last);
}

/**
 * @brief Check if range is strictly sorted, i.e. no 2 elements are equal.
 * @details Uses std::adjacent_find behind the scenes:
 * https://en.cppreference.com/w/cpp/algorithm/adjacent_find.
 *
 * @param first, last The range of the elements to examine.
 * @return Boolean of whether or not the range is strictly sorted.
 */
template<class ForwardIt>
bool is_strictly_sorted(ForwardIt first, ForwardIt last)
{
  return detail::is_sorted_impl<ForwardIt, std::greater_equal>(first, last);
}

/**
 * @brief Check if range is strictly sorted, i.e. no 2 elements are equal.
 * @details Uses std::adjacent_find behind the scenes:
 * https://en.cppreference.com/w/cpp/algorithm/adjacent_find.
 *
 * @param policy Execution policy to use for std::adjacent_find:
 * https://en.cppreference.com/w/cpp/header/execution
 * @param first, last The range of the elements to examine.
 * @return Boolean of whether or not the range is strictly sorted.
 */
template<typename ExecutionPolicy, class ForwardIt>
bool is_strictly_sorted(ExecutionPolicy && policy, ForwardIt first, ForwardIt last)
{
  return detail::is_sorted_impl<ForwardIt, std::greater_equal>(
    std::forward<ExecutionPolicy>(policy), first, last);
}

}  // namespace cbr

#endif  // CBR_UTILS__UTILS_HPP_

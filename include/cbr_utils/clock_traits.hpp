// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__CLOCK_TRAITS_HPP_
#define CBR_UTILS__CLOCK_TRAITS_HPP_

#include <chrono>

namespace cbr::detail
{

/**
 * @brief Simple clock type adapter
 * @details Works out of the box for std::chrono clocks
 *
 */
template<typename clock_t>
struct ClockTraits
{
  using time_point = typename clock_t::time_point;
  using duration = typename clock_t::duration;

  /**
   * @brief Converts clock specific duration type to chono duration type
   * @details Uses std::chrono::duration_cast to do the conversion
   *
   * @param d Duration value
   * @return Converted duration
   */
  template<typename duration_t>
  static duration_t duration_cast(const duration & d)
  {
    return std::chrono::duration_cast<duration_t>(d);
  }
};

}  // namespace cbr::detail

#endif  // CBR_UTILS__CLOCK_TRAITS_HPP_

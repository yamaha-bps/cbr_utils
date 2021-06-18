// Copyright 2020 Yamaha Motor Corporation, USA
#ifndef CBR_UTILS__CLOCK_TRAITS_HPP_
#define CBR_UTILS__CLOCK_TRAITS_HPP_

#include <chrono>

namespace cbr::detail
{

template<typename clock_t>
struct ClockTraits
{
  using time_point = typename clock_t::time_point;
  using duration = typename clock_t::duration;

  template<typename duration_t>
  static duration_t duration_cast(const duration & d)
  {
    return std::chrono::duration_cast<duration_t>(d);
  }
};

}  // namespace cbr::detail

#endif  // CBR_UTILS__CLOCK_TRAITS_HPP_

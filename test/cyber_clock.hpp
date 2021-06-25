// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#include "cbr_utils/clock_traits.hpp"

struct CyberClock
{
  using time_point = std::size_t;
  using duration = std::size_t;

  time_point t = 0;

  time_point now() const
  {
    return t;
  }

  CyberClock & operator+=(duration d)
  {
    t += d;
    return *this;
  }

  CyberClock & operator=(duration d)
  {
    t = d;
    return *this;
  }
};

namespace cbr::detail
{

template<>
struct ClockTraits<CyberClock>
{
  using time_point = typename CyberClock::time_point;
  using duration = typename CyberClock::duration;

  template<typename duration_t>
  static duration_t duration_cast(const duration & d)
  {
    return std::chrono::duration_cast<duration_t>(std::chrono::milliseconds(d));
  }
};

}

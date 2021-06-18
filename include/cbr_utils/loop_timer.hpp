// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_ros/blob/master/LICENSE

#ifndef CBR_UTILS__LOOP_TIMER_HPP_
#define CBR_UTILS__LOOP_TIMER_HPP_

#include <cstdint>
#include <chrono>
#include <numeric>
#include <thread>

namespace cbr
{

template<typename _clock = std::chrono::high_resolution_clock>
class LoopTimer
{
public:
  explicit LoopTimer(const std::chrono::nanoseconds & rate)
  : count_(0),
    rate_(rate),
    tNm1_()
  {}

  void wait()
  {
    if (count_ > 0) {
      auto tTarget = tNm1_ + rate_;
      auto tNow = _clock::now();
      if (tNow < tTarget) {
        auto wait_time = std::chrono::duration_cast<std::chrono::nanoseconds>(tTarget - tNow);
        std::this_thread::sleep_for(wait_time);
        tNm1_ = tTarget;
      } else {
        tNm1_ = tNow;
      }
      count_++;
    } else {
      tNm1_ = _clock::now();
      count_++;
    }
  }

protected:
  uint64_t count_;
  std::chrono::nanoseconds rate_;
  std::chrono::time_point<_clock> tNm1_;
};

}  // namespace cbr

#endif  // CBR_UTILS__LOOP_TIMER_HPP_

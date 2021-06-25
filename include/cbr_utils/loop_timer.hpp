// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__LOOP_TIMER_HPP_
#define CBR_UTILS__LOOP_TIMER_HPP_

#include <chrono>
#include <memory>
#include <numeric>
#include <thread>

#include "clock_traits.hpp"

namespace cbr
{

/**
 * @brief Loop synchronization utility class
 */
template<typename _clock_t = std::chrono::high_resolution_clock, bool _steady = false>
class LoopTimer
{
public:
  using clock_t = _clock_t;
  using time_point_t = typename detail::ClockTraits<clock_t>::time_point;
  using duration_t = typename detail::ClockTraits<clock_t>::duration;

  LoopTimer() = default;
  LoopTimer(const LoopTimer &) = default;
  LoopTimer(LoopTimer &&) = default;
  LoopTimer & operator=(const LoopTimer &) = default;
  LoopTimer & operator=(LoopTimer &&) = default;
  ~LoopTimer() = default;

  explicit LoopTimer(const duration_t & rate) noexcept
  : rate_(rate)
  {}

  LoopTimer(
    const duration_t & rate,
    const std::shared_ptr<clock_t> & clock) noexcept
  : rate_(rate),
    clock_(clock)
  {}

  LoopTimer(
    const duration_t & rate,
    std::shared_ptr<clock_t> && clock) noexcept
  : rate_(rate),
    clock_(std::move(clock))
  {}

  /**
   * @brief set clock
   */
  void set_clock(const std::shared_ptr<clock_t> & clock) noexcept
  {
    clock_ = clock;
  }

  /**
   * @brief set clock
   */
  void set_clock(std::shared_ptr<clock_t> && clock) noexcept
  {
    clock_ = std::move(clock);
  }

  /**
   * @brief set loop rate
   */
  void set_rate(const duration_t & rate) noexcept
  {
    rate_ = rate;
  }

  /**
   * @brief wait until next deadline
   */
  void wait()
  {
    const auto tNow = clock_->now();
    if (count_ > 0) {
      const auto tTarget = tNm1_ + rate_;
      const auto wait_time =
        detail::template
        ClockTraits<clock_t>::template duration_cast<std::chrono::nanoseconds>(tTarget - tNow);
      if constexpr (_steady) {
        tNm1_ = tTarget;
        if (tNow < tTarget) {
          std::this_thread::sleep_for(wait_time);
        }
      } else {
        if (tNow < tTarget) {
          tNm1_ = tTarget;
          std::this_thread::sleep_for(wait_time);
        } else {
          tNm1_ = tNow;
        }
      }
    } else {
      tNm1_ = tNow;
    }
    count_++;
  }

  /**
   * @brief returns how many time the wait() function was called
   */
  const std::size_t & get_count() const noexcept
  {
    return count_;
  }

  /**
   * @brief returns rate
   */
  const duration_t & get_rate() const noexcept
  {
    return rate_;
  }

  /**
   * @brief returns clock
   */
  std::shared_ptr<clock_t> get_clock() const
  {
    return clock_;
  }

protected:
  duration_t rate_{1};
  std::shared_ptr<clock_t> clock_ = std::make_shared<clock_t>();
  time_point_t tNm1_;
  std::size_t count_ = 0;
};

template<typename clock_t = std::chrono::high_resolution_clock>
struct LoopTimerSteady : public LoopTimer<clock_t, true>
{
  LoopTimerSteady() = default;
  LoopTimerSteady(const LoopTimerSteady &) = default;
  LoopTimerSteady(LoopTimerSteady &&) = default;
  LoopTimerSteady & operator=(const LoopTimerSteady &) = default;
  LoopTimerSteady & operator=(LoopTimerSteady &&) = default;
  ~LoopTimerSteady() = default;
  using LoopTimer<clock_t, true>::LoopTimer;
};

}  // namespace cbr

#endif  // CBR_UTILS__LOOP_TIMER_HPP_

// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

/** @file */

#ifndef CBR_UTILS__LOOP_TIMER_HPP_
#define CBR_UTILS__LOOP_TIMER_HPP_

#include <chrono>
#include <memory>
#include <numeric>
#include <thread>

#include "clock_traits.hpp"

namespace cbr {

/**
 * @brief Loop synchronization utility.
 * @details Allows to pace a loop using the wait() member function.
 *
 * Use std::this_thread::sleep_for to sleep for the correct amount of time to pace a loop.
 *
 * If steady functionality is active, and a loop iteration takes longer than timer rate, the timer
 * will try to catch up to yield at times t_i = t_0 + i*rate. If not, and the loop iteration j takes
 * longer than timer rate, then t_i = t_j + i*rate from then on.
 *
 * Example:
 * ```
 * LoopTimer timer;
 *
 * while(true){
 *   timer.wait();
 *   doSomething();
 * }
 * ```
 * Notes:
 * - Clock is default constructed from specified type if not specified at construction or set.
 * - First call to wait() does NOT wait.
 *
 * @tparam clock_t Clock type (default: std::chrono::high_resolution_clock).
 * @tparam steady Boolean value to activate steady functionality (default: false).
 */
template<typename clock_t = std::chrono::high_resolution_clock, bool steady = false>
class LoopTimer
{
public:
  using time_point_t = typename detail::ClockTraits<clock_t>::time_point;
  using duration_t   = typename detail::ClockTraits<clock_t>::duration;

  LoopTimer()                  = default;
  LoopTimer(const LoopTimer &) = default;
  LoopTimer(LoopTimer &&)      = default;
  LoopTimer & operator=(const LoopTimer &) = default;
  LoopTimer & operator=(LoopTimer &&) = default;
  ~LoopTimer()                        = default;

  /**
   * @brief Construct a new LoopTimer object given a loop rate.
   *
   * @param rate Loop rate
   */
  explicit LoopTimer(const duration_t & rate) noexcept : m_rate(rate) {}

  /**
   * @brief Construct a new LoopTimer object given a loop rate and a clock.
   *
   * @param rate Loop rate.
   * @param clock Shared pointer to a clock.
   */
  LoopTimer(const duration_t & rate, const std::shared_ptr<clock_t> & clock) noexcept
      : m_rate(rate), m_clock(clock)
  {}

  /**
   * @brief Construct a new LoopTimer object given a loop rate and a clock.
   *
   * @param rate Loop rate.
   * @param clock Shared pointer to a clock.
   */
  LoopTimer(const duration_t & rate, std::shared_ptr<clock_t> && clock) noexcept
      : m_rate(rate), m_clock(std::move(clock))
  {}

  /**
   * @brief Set the timer's clock.
   *
   * @param clock Shared pointer to a clock.
   */
  void set_clock(const std::shared_ptr<clock_t> & clock) noexcept { m_clock = clock; }

  /**
   * @brief Set the timer's clock.
   *
   * @param clock Shared pointer to a clock.
   */
  void set_clock(std::shared_ptr<clock_t> && clock) noexcept { m_clock = std::move(clock); }

  /**
   * @brief Set the timer's rate.
   *
   * @param rate Timer rate.
   */
  void set_rate(const duration_t & rate) noexcept { m_rate = rate; }

  /**
   * @brief Synchronization function.
   * @details Will block such that the duration between the previous time the function yielded and
   * this time the function will yield is no shorter than the timer's rate.
   */
  void wait()
  {
    const auto tNow = m_clock->now();
    if (m_count > 0) {
      const auto tTarget = m_tNm1 + m_rate;
      const auto wait_time =
        detail::template ClockTraits<clock_t>::template duration_cast<std::chrono::nanoseconds>(
          tTarget - tNow);
      if constexpr (steady) {
        m_tNm1 = tTarget;
        if (tNow < tTarget) { std::this_thread::sleep_for(wait_time); }
      } else {
        if (tNow < tTarget) {
          m_tNm1 = tTarget;
          std::this_thread::sleep_for(wait_time);
        } else {
          m_tNm1 = tNow;
        }
      }
    } else {
      m_tNm1 = tNow;
    }
    m_count++;
  }

  /**
   * @brief Get how many times the wait() function was called.
   *
   * @return How many times the wait() function was called.
   */
  const std::size_t & get_count() const noexcept { return m_count; }

  /**
   * @brief Get timer's rate.
   * @details
   *
   * @return Timer's rate.
   */
  const duration_t & get_rate() const noexcept { return m_rate; }

  /**
   * @brief Get timer's clock.
   *
   * @return Timer's clock.
   */
  std::shared_ptr<clock_t> get_clock() const { return m_clock; }

protected:
  duration_t m_rate{1};
  std::shared_ptr<clock_t> m_clock = std::make_shared<clock_t>();
  time_point_t m_tNm1;
  std::size_t m_count = 0;
};

/**
 * @brief Alias for a steady LoopTimer.
 */
template<typename clock_t = std::chrono::high_resolution_clock>
using LoopTimerSteady = LoopTimer<clock_t, true>;

}  // namespace cbr

#endif  // CBR_UTILS__LOOP_TIMER_HPP_

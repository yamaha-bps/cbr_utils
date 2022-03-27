// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

/** @file */

#ifndef CBR_UTILS__CYBER_TIMER_HPP_
#define CBR_UTILS__CYBER_TIMER_HPP_

#include <chrono>
#include <memory>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include "clock_traits.hpp"

namespace cbr {

/**
 * @brief Timer class with averaging capabilities
 * @details Has a couple of basic functionalities:
 *
 * - tic: Starts the timer.
 * - toc: If timer is started, returns duration since last tic and stops timer. If timer is stopped,
 *     returns same value as last toc. This last toc value can also be retrieved by calling
 *     get_latest().
 * - tac:  Returns duration since last tic, but does NOT stop the timer.
 * - toc_tic: Zero delay call to toc and tic in succession (lap time).
 *
 * If averaging functionality is active, the successive calls to toc (when timer is started) are
 * averaged and this average can be queried by calling get_average(), or reset by calling restart().
 *
 * Example usage:
 * ```
 * CyberTimer<> timer;
 * timer.tic();
 * complexFunctionTakingTime1();
 * const auto duration1 = timer.tac();
 * complexFunctionTakingTime2();
 * const auto duration2 = timer.toc();
 *
 * std::cout << "Function 1 took: " << duration1 << "seconds" << std::endl;
 * std::cout << "Functions 1 and 2 combined took: " << duration2 << "seconds" << std::endl;
 * ```
 * Notes:
 * clock is default constructed from specified type if not specified at construction or set.
 *
 * @tparam ratio_t Duration units (default: std::ratio<1>, i.e. seconds)
 * @tparam T Duration underlying representation type (default: double)
 * @tparam clock_t Clock type (default: std::chrono::high_resolution_clock)
 * @tparam with_average Boolean value to activate averaging functionalities (default: true)
 */
template<typename ratio_t = std::ratio<1>,
  typename T              = double,
  typename clock_t        = std::chrono::high_resolution_clock,
  bool with_average       = true>
class CyberTimer
{
  static_assert(
    std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, "Type must be arithmetic but not bool.");

public:
  using duration_t   = std::chrono::duration<T, ratio_t>;
  using time_point_t = typename detail::ClockTraits<clock_t>::time_point;

  CyberTimer()                   = default;
  CyberTimer(const CyberTimer &) = default;
  CyberTimer(CyberTimer &&)      = default;
  CyberTimer & operator=(const CyberTimer &) = default;
  CyberTimer & operator=(CyberTimer &&) = default;
  ~CyberTimer()                         = default;

  /**
   * @brief Construct a new CyberTimer object.
   *
   * @param clock Shared pointer to a clock.
   */
  explicit CyberTimer(const std::shared_ptr<clock_t> & clock) noexcept : clock_(clock) {}

  /**
   * @brief Construct a new CyberTimer object.
   *
   * @param clock Shared pointer to a clock.
   */
  explicit CyberTimer(std::shared_ptr<clock_t> && clock) noexcept : clock_(std::move(clock)) {}

  /**
   * @brief Set the clock object.
   *
   * @param clock Shared pointer to a clock.
   */
  void set_clock(const std::shared_ptr<clock_t> & clock) noexcept { clock_ = clock; }

  /**
   * @brief Set the clock object.
   *
   * @param clock Shared pointer to a clock.
   */
  void set_clock(std::shared_ptr<clock_t> && clock) noexcept { clock_ = std::move(clock); }

  /**
   * @brief Current clock time.
   * @details calls clock.now().
   *
   * @return Timepoint for current clock time.
   */
  time_point_t now() const noexcept { return clock_->now(); }

  /**
   * @brief Starts timer to specified timepoint.
   *
   * @param t_start Timepoint to set timer to.
   */
  void tic(const time_point_t t_start) noexcept
  {
    t_start_ = t_start;
    running_ = true;
  }

  /**
   * @brief Starts timer to current clock time.
   *
   */
  void tic() noexcept { tic(clock_->now()); }

  /**
   * @brief Returns duration between specified timepoint and when timer was last started.
   * @details Does not stop the timer. Undefined behavior if called before timer was ever started.
   *
   * @param t_now Timepoint to use for duration computation.
   * @return Duration between specified timepoint and when timer was last started.
   */
  duration_t tac_chrono(const time_point_t t_now) const noexcept
  {
    const auto duration = t_now - t_start_;
    return detail::template ClockTraits<clock_t>::template duration_cast<duration_t>(duration);
  }

  /**
   * @brief Returns duration between current clock timepoint and when timer was last started.
   * @details Does not stop the timer. Undefined behavior if called before timer was ever started.
   *
   * @return Duration between current clock timepoint and when timer was last started.
   */
  duration_t tac_chrono() const noexcept { return tac_chrono(clock_->now()); }

  /**
   * @brief Returns duration between specified timepoint and when timer was last started.
   * @details Does not stop the timer. Undefined behavior if called before timer was ever started.
   *
   * @param t_now Timepoint to use for duration computation.
   * @return Duration between specified timepoint and when timer was last started.
   */
  T tac(const time_point_t t_now) const noexcept { return tac_chrono(t_now).count(); }

  /**
   * @brief Returns duration between current clock timepoint and when timer was last started.
   * @details Does not stop the timer. Undefined behavior if called before timer was ever started.
   *
   * @return Duration between current clock timepoint and when timer was last started.
   */
  T tac() const noexcept { return tac_chrono().count(); }

  /**
   * @brief Stops timer at specified time.
   * @details  If with_average==true, updates the timer duration average.
   * Returns latest timer duration if timer is not running when called and does not update
   * the timer duration average.
   * Undefined behavior if called before timer was ever started.
   *
   * @param t_stop Timepoint at which timer is stopped.
   * @return Duration between specified timepoint and when timer was last started.
   */
  const duration_t & toc_chrono(const time_point_t t_stop) noexcept
  {
    if (running_) {
      running_ = false;
      dt_      = tac_chrono(t_stop);
      if constexpr (with_average) {
        i_++;
        compute_avg();
      }
    }

    return dt_;
  }

  /**
   * @brief Stops timer at current clock time.
   * @details  If with_average==true, updates the timer duration average.
   * Returns latest timer duration if timer is not running when called and does not update
   * the timer duration average.
   * Undefined behavior if called before timer was ever started.
   *
   * @return Duration between current clock timepoint and when timer was last started.
   */
  const duration_t & toc_chrono() noexcept { return toc_chrono(clock_->now()); }

  /**
   * @brief Zero delay stop and restart of timer at specified time.
   * @details Undefined behavior if called before timer was ever started.
   *
   * @param t_stop Timepoint at which timer is stopped and restarted.
   * @return Duration between specified timepoint and when timer was last started.
   */
  const duration_t & toc_tic_chrono(const time_point_t t_stop) noexcept
  {
    toc_chrono(t_stop);
    tic(t_stop);
    return dt_;
  }

  /**
   * @brief Zero delay stop and restart of timer at current clock time.
   * @details Undefined behavior if called before timer was ever started.
   *
   * @return Duration between current clock timepoint and when timer was last started.
   */
  const duration_t & toc_tic_chrono() noexcept { return toc_tic_chrono(clock_->now()); }

  /**
   * @brief Stops timer at specified time.
   * @details  If with_average==true, updates the timer duration average.
   * Returns latest timer duration if timer is not running when called and does not update
   * the timer duration average.
   * Undefined behavior if called before timer was ever started.
   *
   * @param t_stop Timepoint at which timer is stopped.
   * @return Duration between specified timepoint and when timer was last started.
   */
  T toc(const time_point_t t_stop) noexcept { return toc_chrono(t_stop).count(); }

  /**
   * @brief Stops timer at current clock time.
   * @details  If with_average==true, updates the timer duration average.
   * Returns latest timer duration if timer is not running when called and does not update
   * the timer duration average.
   * Undefined behavior if called before timer was ever started.
   *
   * @return Duration between current clock timepoint and when timer was last started.
   */
  T toc() noexcept { return toc_chrono().count(); }

  /**
   * @brief Zero delay stop and restart of timer at specified time.
   * @details Undefined behavior if called before timer was ever started.
   *
   * @param t_stop Timepoint at which timer is stopped and restarted.
   * @return Duration between specified timepoint and when timer was last started.
   */
  T toc_tic(const time_point_t t_stop) noexcept { return toc_tic_chrono(t_stop).count(); }

  /**
   * @brief Zero delay stop and restart of timer at current clock time.
   * @details Undefined behavior if called before timer was ever started.
   *
   * @return Duration between current clock timepoint and when timer was last started.
   */
  T toc_tic() noexcept { return toc_tic_chrono().count(); }

  /**
   * @brief Stops timer without updating duration average and latest duration.
   */
  void stop() noexcept { running_ = false; }

  /**
   * @brief Resets the timer duration average and restarts clock to specified time.
   * @details Only available if with_average==true.
   *
   * @param t_start Timepoint to reset timer to.
   */
  template<typename _T = void>
  std::enable_if_t<with_average, _T> restart(const time_point_t t_start) noexcept
  {
    avg_ = 0.;
    i_   = 0;
    tic(t_start);
  }

  /**
   * @brief Resets the timer duration average and restarts clock to current clock time
   * @details Only available if with_average==true.
   */
  template<typename _T = void>
  std::enable_if_t<with_average, _T> restart() noexcept
  {
    restart(clock_->now());
  }

  /**
   * @brief Get number of duration samples used for the computation of the average timer duration.
   * @details Number since construction or last restart Only available if with_average==true.
   *
   * @return Number of duration samples used for the computation of the average timer duration.
   */
  template<typename _T = const std::size_t &>
  std::enable_if_t<with_average, _T> get_average_count() const noexcept
  {
    return i_;
  }

  /**
   * @brief Get average timer duration.
   * @details As a double, in the time unit specified by ratio_t, for all pairs of tic() and toc()
   * calls since construction or last restart. Only available if with_average==true. if
   * getAverageCount()==0, returns 0.
   *
   * @return Average duration over all pairs of successive tic and toc calls.
   */
  template<typename _T = const double &>
  std::enable_if_t<with_average, _T> get_average() const noexcept
  {
    return avg_;
  }

  /**
   * @brief Get whether or not the timer is running.
   * @details Is always true just after a tic() call, and walays false just after a toc() call.
   *
   * @return Average duration over all pairs of successive tic and toc calls.
   */
  const auto & is_runnning() const noexcept { return running_; }

  /**
   * @brief Get latest timer duration.
   * @details Undefined behavior if called before timer was ever started.
   *
   * @return Duration over latest successive tic and toc calls.
   */
  const auto & get_latest_chrono() const noexcept { return dt_; }

  /**
   * @brief Get latest timer duration.
   * @details Undefined behavior if called before timer was ever started.
   *
   * @return Duration over latest successive tic and toc calls.
   */
  auto get_latest() const noexcept { return dt_.count(); }

  /**
   * @brief Get timer's clock.
   *
   * @return Timer's clock.
   */
  std::shared_ptr<clock_t> get_clock() const { return clock_; }

protected:
  /**
   * @brief Update average timer duration value
   */
  template<typename _T = void>
  std::enable_if_t<with_average, _T> compute_avg() noexcept
  {
    avg_ = (static_cast<double>(i_ - 1) * avg_ + static_cast<double>(dt_.count()))
         / static_cast<double>(i_);
  }

protected:
  std::shared_ptr<clock_t> clock_ = std::make_shared<clock_t>();
  std::size_t i_                  = 0;
  duration_t dt_{};
  double avg_   = 0.;
  bool running_ = false;
  time_point_t t_start_{};
};

// /**
//  * @brief CyberTimer template without average computation
//  */
template<typename ratio_t = std::ratio<1>,
  typename T              = double,
  typename clock_t        = std::chrono::high_resolution_clock>
using CyberTimerNoAvg = CyberTimer<ratio_t, T, clock_t, false>;

/**
 * @brief CyberTimer template with milliseconds units and int64_t default duration representation
 */
template<typename T = int64_t,
  typename clock_t  = std::chrono::high_resolution_clock,
  bool with_average = true>
using CyberTimerMilli = CyberTimer<std::milli, T, clock_t, with_average>;

/**
 * @brief CyberTimer template with microseconds units and int64_t default duration representation
 */
template<typename T = int64_t,
  typename clock_t  = std::chrono::high_resolution_clock,
  bool with_average = true>
using CyberTimerMicro = CyberTimer<std::micro, T, clock_t, with_average>;

/**
 * @brief CyberTimer template with nanoseconds units and int64_t default duration representation
 */
template<typename T = int64_t,
  typename clock_t  = std::chrono::high_resolution_clock,
  bool with_average = true>
using CyberTimerNano = CyberTimer<std::nano, T, clock_t, with_average>;

}  // namespace cbr

#endif  // CBR_UTILS__CYBER_TIMER_HPP_

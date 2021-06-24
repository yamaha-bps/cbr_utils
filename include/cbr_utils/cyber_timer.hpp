// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_ros/blob/master/LICENSE

#ifndef CBR_UTILS__CYBER_TIMER_HPP_
#define CBR_UTILS__CYBER_TIMER_HPP_

#include <chrono>
#include <memory>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include "clock_traits.hpp"

namespace cbr
{

/**
 * @brief
 *
 */
template<
  typename ratio_t = std::ratio<1>,
  typename T = double,
  typename _clock_t = std::chrono::high_resolution_clock,
  bool with_average = true
>
class CyberTimer
{
  static_assert(
    std::is_arithmetic_v<T>&& !std::is_same_v<T, bool>,
    "Type must be arithmetic but not bool.");

public:
  using clock_t = _clock_t;
  using duration_t = std::chrono::duration<T, ratio_t>;
  using time_point_t = typename detail::ClockTraits<clock_t>::time_point;

  CyberTimer() = default;
  CyberTimer(const CyberTimer &) = default;
  CyberTimer(CyberTimer &&) = default;
  CyberTimer & operator=(const CyberTimer &) = default;
  CyberTimer & operator=(CyberTimer &&) = default;
  ~CyberTimer() = default;

  explicit CyberTimer(const std::shared_ptr<clock_t> & clock) noexcept
  : clock_(clock)
  {}

  explicit CyberTimer(std::shared_ptr<clock_t> && clock) noexcept
  : clock_(std::move(clock))
  {}

  void set_clock(const std::shared_ptr<clock_t> & clock) noexcept
  {
    clock_ = clock;
  }

  void set_clock(std::shared_ptr<clock_t> && clock) noexcept
  {
    clock_ = std::move(clock);
  }

  /**
   * @brief returns current clock time
   *
   */
  time_point_t now() const noexcept
  {
    return clock_->now();
  }

  /**
   * @brief starts timer to specified timepoint
   *
   */
  void tic(const time_point_t t_start) noexcept
  {
    t_start_ = t_start;
    running_ = true;
  }

  /**
   * @brief starts timer to current clock time
   *
   */
  void tic() noexcept
  {
    tic(clock_->now());
  }

  /**
   * @brief returns duration between specified timepoint and when timer last started.
   * Undefined behavior if called before timer was ever started
   */
  duration_t tacChrono(const time_point_t t_stop) const noexcept
  {
    const auto duration = t_stop - t_start_;
    return detail::template ClockTraits<clock_t>::template duration_cast<duration_t>(duration);
  }

  /**
   * @brief returns duration between current clock time and when timer last started.
   * Undefined behavior if called before timer was ever started
   */
  duration_t tacChrono() const noexcept
  {
    return tacChrono(clock_->now());
  }

  /**
   * @brief returns duration count between specified timepoint and when timer last started.
   * Undefined behavior if called before timer was ever started
   */
  T tac(const time_point_t t_stop) const noexcept
  {
    return tacChrono(t_stop).count();
  }

  /**
   * @brief returns duration count between current clock time and when timer last started.
   * Undefined behavior if called before timer was ever started
   */
  T tac() const noexcept
  {
    return tacChrono().count();
  }

  /**
   * @brief stops timer and returns duration between specified timepoint
   * and when timer last started.
   * If with_average==true, updates the timer duration average
   * Returns latest timer duration if timer is not running when called and does not update
   * the timer duration average.
   * Undefined behavior if called before timer was ever started
   */
  const duration_t & tocChrono(const time_point_t t_stop) noexcept
  {
    if (running_) {
      dt_ = tacChrono(t_stop);
      if constexpr (with_average) {
        i_++;
        compute_avg();
      }
    }

    return dt_;
  }

  /**
   * @brief stops timer and returns duration between current clock time
   * and when timer last started.
   * If with_average==true, updates the timer duration average
   */
  const duration_t & tocChrono() noexcept
  {
    return tocChrono(clock_->now());
  }

  /**
   * @brief calls tocChrono and then tic with specified timepoint
   * and returns the result of the tocChrono call
   */
  const duration_t & tocticChrono(const time_point_t t_stop) noexcept
  {
    tocChrono(t_stop);
    tic(t_stop);
    return dt_;
  }

  /**
   * @brief calls tocChrono and then tic with the same current clock time
   * and returns the result of the tocChrono call
   */
  const duration_t & tocticChrono() noexcept
  {
    return tocticChrono(clock_->now());
  }

  /**
   * @brief stops timer and returns duration count between specified timepoint
   * and when timer last started.
   * If with_average==true, updates the timer duration average
   */
  T toc(const time_point_t t_stop) noexcept
  {
    return tocChrono(t_stop).count();
  }

  /**
   * @brief stops timer and returns duration count between current clock time
   * and when timer last started.
   * If with_average==true, updates the timer duration average
   */
  T toc() noexcept
  {
    return tocChrono().count();
  }

  /**
   * @brief calls toc and then tic with specified timepoint
   * and returns the result of the toc call
   */
  T toctic(const time_point_t t_stop) noexcept
  {
    return tocticChrono(t_stop).count();
  }

  /**
   * @brief calls toc and then tic with the same current clock time
   * and returns the result of the toc call
   */
  T toctic() noexcept
  {
    return tocticChrono().count();
  }

  /**
   * @brief stops timer without updating duration average and latest duration.
   *
   */
  void stop() noexcept
  {
    running_ = false;
  }

  /**
   * @brief resets the timer duration average and restarts clock to specified timepoint
   * Only available if with_average==true.
   */
  template<typename _T = void>
  std::enable_if_t<with_average, _T> restart(const time_point_t t_start) noexcept
  {
    avg_ = 0.;
    i_ = 0;
    tic(t_start);
  }

  /**
   * @brief resets the timer duration average and restarts clock to current clock time
   * Only available if with_average==true.
   */
  template<typename _T = void>
  std::enable_if_t<with_average, _T> restart() noexcept
  {
    restart(clock_->now());
  }

  /**
   * @brief get number of duration samples used for the computation of the average timer duration
   * since construction or last restart.
   * Only available if with_average==true.
   */
  template<typename _T = const std::size_t &>
  std::enable_if_t<with_average, _T> getAverageCount() const noexcept
  {
    return i_;
  }

  /**
   * @brief get average timer duration (as a double, in the time unit specified by ratio_t)
   * for all pairs of tic() and toc() calls since construction or last restart.
   * Only available if with_average==true.
   * if getAverageCount()==0, returns 0.
   */
  template<typename _T = const double &>
  std::enable_if_t<with_average, _T> getAverage() const noexcept
  {
    return avg_;
  }


  /**
   * @brief returns whether or not the timer is running,
   * true just after a tic() call, and false just after a toc() call
   */
  const auto & isRunnning() const noexcept
  {
    return running_;
  }

  /**
   * @brief returns latest timer duration
   * Undefined behavior if called before timer was ever started
   */
  const auto & getLatestChrono() const noexcept
  {
    return dt_;
  }

  /**
   * @brief returns latest timer duration count
   * Undefined behavior if called before timer was ever started
   */
  auto getLatestCount() const noexcept
  {
    return dt_.count();
  }

protected:
  template<typename _T = void>
  std::enable_if_t<with_average, _T> compute_avg() noexcept
  {
    avg_ = (static_cast<double>(i_ - 1) * avg_ + static_cast<double>(dt_.count())) /
      static_cast<double>(i_);
  }

protected:
  std::shared_ptr<clock_t> clock_ = std::make_shared<clock_t>();
  std::size_t i_ = 0;
  duration_t dt_{};
  double avg_ = 0.;
  bool running_ = false;
  time_point_t t_start_{};
};

/**
 * @brief CyberTimer template without average computation
 */
template<
  typename ratio_t = std::ratio<1>,
  typename T = double,
  typename clock_t = std::chrono::high_resolution_clock
>
struct CyberTimerNoAvg : public CyberTimer<ratio_t, T, clock_t, false>
{
  CyberTimerNoAvg() = default;
  CyberTimerNoAvg(const CyberTimerNoAvg &) = default;
  CyberTimerNoAvg(CyberTimerNoAvg &&) = default;
  CyberTimerNoAvg & operator=(const CyberTimerNoAvg &) = default;
  CyberTimerNoAvg & operator=(CyberTimerNoAvg &&) = default;
  ~CyberTimerNoAvg() = default;
  using CyberTimer<ratio_t, T, clock_t, false>::CyberTimer;
};

/**
 * @brief CyberTimer template with milliseconds units and int64_t default duration representation
 */
template<
  typename T = int64_t,
  typename clock_t = std::chrono::high_resolution_clock,
  bool with_average = true
>
struct CyberTimerMilli : public CyberTimer<std::milli, T, clock_t, with_average>
{
  CyberTimerMilli() = default;
  CyberTimerMilli(const CyberTimerMilli &) = default;
  CyberTimerMilli(CyberTimerMilli &&) = default;
  CyberTimerMilli & operator=(const CyberTimerMilli &) = default;
  CyberTimerMilli & operator=(CyberTimerMilli &&) = default;
  ~CyberTimerMilli() = default;
  using CyberTimer<std::milli, T, clock_t, with_average>::CyberTimer;
};

/**
 * @brief CyberTimer template with microseconds units and int64_t default duration representation
 */
template<
  typename T = int64_t,
  typename clock_t = std::chrono::high_resolution_clock,
  bool with_average = true
>
struct CyberTimerMicro : public CyberTimer<std::micro, T, clock_t, with_average>
{
  CyberTimerMicro() = default;
  CyberTimerMicro(const CyberTimerMicro &) = default;
  CyberTimerMicro(CyberTimerMicro &&) = default;
  CyberTimerMicro & operator=(const CyberTimerMicro &) = default;
  CyberTimerMicro & operator=(CyberTimerMicro &&) = default;
  ~CyberTimerMicro() = default;
  using CyberTimer<std::micro, T, clock_t, with_average>::CyberTimer;
};

/**
 * @brief CyberTimer template with nanoseconds units and int64_t default duration representation
 */
template<
  typename T = int64_t,
  typename clock_t = std::chrono::high_resolution_clock,
  bool with_average = true
>
struct CyberTimerNano : public CyberTimer<std::nano, T, clock_t, with_average>
{
  CyberTimerNano() = default;
  CyberTimerNano(const CyberTimerNano &) = default;
  CyberTimerNano(CyberTimerNano &&) = default;
  CyberTimerNano & operator=(const CyberTimerNano &) = default;
  CyberTimerNano & operator=(CyberTimerNano &&) = default;
  ~CyberTimerNano() = default;
  using CyberTimer<std::nano, T, clock_t, with_average>::CyberTimer;
};

}  // namespace cbr

#endif  // CBR_UTILS__CYBER_TIMER_HPP_

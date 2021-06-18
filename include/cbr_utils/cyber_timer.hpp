// Copyright 2020 Yamaha Motor Corporation, USA
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
  using time_point = typename detail::ClockTraits<clock_t>::time_point;

  CyberTimer() = default;
  CyberTimer(const CyberTimer &) = default;
  CyberTimer(CyberTimer &&) = default;
  CyberTimer & operator=(const CyberTimer &) = default;
  CyberTimer & operator=(CyberTimer &&) = default;
  ~CyberTimer() = default;

  explicit CyberTimer(std::shared_ptr<clock_t> clock) noexcept
  : clock_(std::move(clock))
  {}

  explicit CyberTimer(const clock_t & clock)
  : clock_(std::make_shared<clock_t>(clock))
  {}

  explicit CyberTimer(clock_t && clock)
  : clock_(std::make_shared<clock_t>(std::move(clock)))
  {}

  void set_clock(std::shared_ptr<clock_t> clock) noexcept
  {
    clock_ = std::move(clock);
  }

  void set_clock(const clock_t & clock)
  {
    clock_ = std::make_shared<clock_t>(clock);
  }

  void set_clock(clock_t && clock)
  {
    clock_ = std::make_shared<clock_t>(std::move(clock));
  }

  time_point now() const noexcept
  {
    return clock_->now();
  }

  void tic(const time_point t_start) noexcept
  {
    t_start_ = t_start;
    running_ = true;
  }

  void tic() noexcept
  {
    tic(clock_->now());
  }

  duration_t tacChrono(const time_point t_stop) const noexcept
  {
    const auto duration = t_stop - t_start_;
    return detail::template ClockTraits<clock_t>::template duration_cast<duration_t>(duration);
  }

  duration_t tacChrono() const noexcept
  {
    return tacChrono(clock_->now());
  }

  T tac(const time_point t_stop) const noexcept
  {
    return tacChrono(t_stop).count();
  }

  T tac() const noexcept
  {
    return tacChrono().count();
  }

  const duration_t & tocChrono(const time_point t_stop) noexcept
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

  const duration_t & tocChrono() noexcept
  {
    return tocChrono(clock_->now());
  }

  const duration_t & tocticChrono(const time_point t_stop) noexcept
  {
    tocChrono(t_stop);
    tic(t_stop);
    return dt_;
  }

  const duration_t & tocticChrono() noexcept
  {
    return tocticChrono(clock_->now());
  }

  T toc(const time_point t_stop) noexcept
  {
    return tocChrono(t_stop).count();
  }

  T toc() noexcept
  {
    return tocChrono().count();
  }

  T toctic(const time_point t_stop) noexcept
  {
    return tocticChrono(t_stop).count();
  }

  T toctic() noexcept
  {
    return tocticChrono().count();
  }

  void restart(const time_point t_start) noexcept
  {
    avg_ = 0.;
    if constexpr (with_average) {
      i_ = 0;
    }
    tic(t_start);
  }

  void restart() noexcept
  {
    restart(clock_->now());
  }

  void stop() noexcept
  {
    running_ = false;
  }

  template<typename _T = const double &>
  std::enable_if_t<with_average, _T> getAverage() const noexcept
  {
    return avg_;
  }

  template<typename _T = const std::size_t &>
  std::enable_if_t<with_average, _T> getAverageCount() const noexcept
  {
    return i_;
  }

  const auto & isRunnning() const noexcept
  {
    return running_;
  }

  const auto & getLatestChrono() const noexcept
  {
    return dt_;
  }

  auto getLatest() const noexcept
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
  std::size_t i_{};
  duration_t dt_{};
  double avg_{};
  bool running_{};
  time_point t_start_{};
};

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

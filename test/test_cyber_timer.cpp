// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#include <gtest/gtest.h>

#include <vector>

#include "cbr_utils/cyber_timer.hpp"

#include "cyber_clock.hpp"

using cbr::CyberTimer;
using cbr::CyberTimerMilli;

TEST(CyberTimer, Init)
{
  CyberTimer timer1;

  auto clock1 = std::make_shared<std::chrono::high_resolution_clock>();
  CyberTimer timer3(clock1);
  ASSERT_EQ(clock1, timer3.get_clock());

  CyberTimer timer4(std::move(clock1));
  ASSERT_NE(clock1, timer4.get_clock());

  auto clock2 = std::make_shared<std::chrono::high_resolution_clock>();
  ASSERT_NE(clock2, timer1.get_clock());
  timer1.set_clock(clock2);
  ASSERT_EQ(clock2, timer1.get_clock());

  auto clock3 = std::make_shared<std::chrono::high_resolution_clock>();
  auto clock4 = clock3;
  timer1.set_clock(std::move(clock3));
  ASSERT_NE(clock3, timer1.get_clock());
  ASSERT_EQ(clock4, timer1.get_clock());
}

TEST(CyberTimer, Basic)
{
  constexpr std::size_t N = 100;
  std::vector<double> buffer;
  buffer.reserve(N);

  auto clock = std::make_shared<CyberClock>();

  CyberTimer<std::micro, double, CyberClock> tmr(clock);
  ASSERT_EQ(tmr.is_runnning(), false);

  ASSERT_EQ(tmr.now(), 0);
  *clock += 10;
  ASSERT_EQ(tmr.now(), 10);

  for (std::size_t i = 0; i < N; i++) {
    tmr.tic(clock->now());
    *clock += 10;
    const auto & dt = tmr.toc(clock->now());

    buffer.emplace_back(dt);
  }

  const double avg =
    std::accumulate(buffer.begin(), buffer.end(), 0.) /
    static_cast<double>(tmr.get_average_count());

  ASSERT_NEAR(avg, tmr.get_average(), 1e-8);
  ASSERT_NEAR(tmr.get_average(), 10000., 1000.);
  ASSERT_EQ(tmr.get_average_count(), N);

  ASSERT_EQ(tmr.is_runnning(), false);

  tmr.restart();

  ASSERT_EQ(tmr.is_runnning(), true);
  ASSERT_EQ(tmr.get_average_count(), 0LU);
  ASSERT_EQ(tmr.get_average(), 0.);

  tmr.tic();
  *clock += 100;
  const auto dt1 = tmr.toc();
  ASSERT_EQ(tmr.is_runnning(), false);
  const auto dt2 = tmr.toc_tic();
  ASSERT_EQ(tmr.is_runnning(), true);
  const auto dt3 = tmr.get_latest();
  const auto dt4 = tmr.get_latest_chrono().count();

  ASSERT_EQ(dt1, dt2);
  ASSERT_EQ(dt2, dt3);
  ASSERT_EQ(dt3, dt4);

  tmr.stop();
  ASSERT_EQ(tmr.is_runnning(), false);
  ASSERT_EQ(tmr.get_average_count(), 1LU);
  ASSERT_NEAR(tmr.get_average(), 100000., 10000.);

  *clock += 10;
  ASSERT_DOUBLE_EQ(tmr.tac(), 10000.);
}

// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_ros/blob/master/LICENSE

#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "cbr_utils/cyber_timer.hpp"

using namespace std::chrono_literals;

TEST(CyberTimer, staticWithAverage)
{
  using cbr::CyberTimer;
  using cbr::CyberTimerMilli;

  constexpr std::size_t N = 100;
  std::vector<double> buffer;
  buffer.reserve(N);

  CyberTimer<std::micro> tmr;
  ASSERT_EQ(tmr.isRunnning(), false);

  for (std::size_t i = 0; i < N; i++) {
    tmr.tic();
    std::this_thread::sleep_for(10ms);
    const auto & dt = tmr.toc();

    buffer.emplace_back(dt);
  }

  const double avg =
    std::accumulate(buffer.begin(), buffer.end(), 0.) / static_cast<double>(tmr.getAverageCount());

  ASSERT_NEAR(avg, tmr.getAverage(), 1e-8);
  ASSERT_NEAR(tmr.getAverage(), 10000., 1000.);
  ASSERT_EQ(tmr.getAverageCount(), N);

  ASSERT_EQ(tmr.isRunnning(), true);

  tmr.restart();

  ASSERT_EQ(tmr.isRunnning(), true);
  ASSERT_EQ(tmr.getAverageCount(), 0LU);
  ASSERT_EQ(tmr.getAverage(), 0.);

  tmr.tic();
  std::this_thread::sleep_for(100ms);
  tmr.toc();

  tmr.stop();
  ASSERT_EQ(tmr.isRunnning(), false);
  ASSERT_EQ(tmr.getAverageCount(), 1LU);
  ASSERT_NEAR(tmr.getAverage(), 100000., 10000.);
}

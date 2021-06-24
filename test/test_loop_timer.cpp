// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_ros/blob/master/LICENSE

#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "cbr_utils/loop_timer.hpp"

using namespace std::chrono_literals;

using cbr::LoopTimer;
using cbr::LoopTimerSteady;

TEST(LoopTimer, Constructors)
{
  LoopTimer timer1{};
  LoopTimer timer2(1s);
  const auto clock = std::make_shared<std::chrono::high_resolution_clock>();
  LoopTimer timer3(1ms, clock);
  LoopTimer timer4(1ms, std::make_shared<std::chrono::high_resolution_clock>());
}

TEST(LoopTimer, WaitNotSteady)
{
  LoopTimer timer(100ms);

  const auto t0 = std::chrono::high_resolution_clock::now();
  timer.wait();
  const auto t1 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t1 - t0).count(), 0, 1000000);

  timer.wait();
  const auto t2 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t2 - t1).count(), 100000000, 1000000);

  std::this_thread::sleep_for(50ms);
  timer.wait();
  const auto t3 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t3 - t2).count(), 100000000, 1000000);

  std::this_thread::sleep_for(150ms);
  timer.wait();
  const auto t4 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t4 - t3).count(), 150000000, 1000000);

  std::this_thread::sleep_for(30ms);
  timer.wait();
  const auto t5 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t5 - t4).count(), 100000000, 1000000);
}

TEST(LoopTimer, WaitSteady)
{
  LoopTimerSteady<> timer(100ms);

  const auto t0 = std::chrono::high_resolution_clock::now();
  timer.wait();
  const auto t1 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t1 - t0).count(), 0, 1000000);

  timer.wait();
  const auto t2 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t2 - t1).count(), 100000000, 1000000);

  std::this_thread::sleep_for(50ms);
  timer.wait();
  const auto t3 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t3 - t2).count(), 100000000, 1000000);

  std::this_thread::sleep_for(150ms);
  timer.wait();
  const auto t4 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t4 - t3).count(), 150000000, 1000000);

  timer.wait();
  const auto t5 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t5 - t4).count(), 50000000, 1000000);

  std::this_thread::sleep_for(150ms);
  timer.wait();
  const auto t6 = std::chrono::high_resolution_clock::now();
  std::this_thread::sleep_for(30ms);
  timer.wait();
  const auto t7 = std::chrono::high_resolution_clock::now();
  ASSERT_NEAR((t7 - t6).count(), 50000000, 1000000);
}

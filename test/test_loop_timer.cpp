// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "cbr_utils/loop_timer.hpp"

using namespace std::chrono_literals;

using cbr::LoopTimer;
using cbr::LoopTimerSteady;

constexpr int64_t eps = 2000000;

TEST(LoopTimer, Init)
{
  LoopTimer timer1{};
  ASSERT_EQ(timer1.get_rate(), 1ns);

  LoopTimer timer2(1s);
  ASSERT_EQ(timer2.get_rate(), 1s);
  timer2.set_rate(10ms);
  ASSERT_EQ(timer2.get_rate(), 10ms);

  auto clock1 = std::make_shared<std::chrono::high_resolution_clock>();
  LoopTimer timer3(1ms, clock1);
  ASSERT_EQ(clock1, timer3.get_clock());

  LoopTimer timer4(1ms, std::move(clock1));
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

TEST(LoopTimer, WaitNotSteady)
{
  LoopTimer timer(100ms);
  ASSERT_EQ(timer.get_count(), 0);

  const auto t0 = std::chrono::high_resolution_clock::now();
  timer.wait();
  ASSERT_EQ(timer.get_count(), 1);
  const auto t1 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t1 - t0).count() - 0) < eps);

  timer.wait();
  ASSERT_EQ(timer.get_count(), 2);
  const auto t2 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t2 - t1).count() - 100000000) < eps);

  std::this_thread::sleep_for(50ms);
  timer.wait();
  ASSERT_EQ(timer.get_count(), 3);
  const auto t3 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t3 - t2).count() - 100000000) < eps);

  std::this_thread::sleep_for(150ms);
  timer.wait();
  ASSERT_EQ(timer.get_count(), 4);
  const auto t4 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t4 - t3).count() - 150000000) < eps);

  std::this_thread::sleep_for(30ms);
  timer.wait();
  ASSERT_EQ(timer.get_count(), 5);
  const auto t5 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t5 - t4).count() - 100000000) < eps);
}

TEST(LoopTimer, WaitSteady)
{
  LoopTimerSteady<> timer(100ms);

  const auto t0 = std::chrono::high_resolution_clock::now();
  timer.wait();
  const auto t1 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t1 - t0).count() - 0) < eps);

  timer.wait();
  const auto t2 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t2 - t1).count() - 100000000) < eps);

  std::this_thread::sleep_for(50ms);
  timer.wait();
  const auto t3 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t3 - t2).count() - 100000000) < eps);

  std::this_thread::sleep_for(150ms);
  timer.wait();
  const auto t4 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t4 - t3).count() - 150000000) < eps);

  timer.wait();
  const auto t5 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t5 - t4).count() - 50000000) < eps);

  std::this_thread::sleep_for(150ms);
  timer.wait();
  const auto t6 = std::chrono::high_resolution_clock::now();
  std::this_thread::sleep_for(30ms);
  timer.wait();
  const auto t7 = std::chrono::high_resolution_clock::now();
  ASSERT_TRUE(std::abs((t7 - t6).count() - 50000000) < eps);
}

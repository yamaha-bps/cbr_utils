// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#include <gtest/gtest.h>

#include <array>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "cbr_utils/thread_pool.hpp"

TEST(ThreadPool, main)
{
  using cbr::ThreadPool;
  ThreadPool pool(2);
}

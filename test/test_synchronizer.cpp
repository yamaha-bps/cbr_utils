// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cbr_utils/synchronizer.hpp"

TEST(SynchronizerTest, Print)
{
  cbr::Synchronizer<int, std::string, std::string> sync;

  sync.set_time_fcn<0>([](const int & i) { return i; });
  sync.set_time_fcn<1>([](const std::string & s) { return s.size(); });
  sync.set_time_fcn<2>([](const std::string & s) { return 2 * s.size(); });

  sync.add_and_search<0>(3);
  sync.add_and_search<0>(7);
  sync.add_and_search<0>(12);

  std::stringstream ss;

  ss << sync << std::endl;

  sync.add_and_search<1>("hello");
  sync.add_and_search<1>("merhaba");

  ss << sync << std::endl;

  sync.add_and_search<1>("hello hello");
  sync.add_and_search<2>("hello");

  ss << sync << std::endl;
}

TEST(SynchronizerTest, Basic)
{
  cbr::Synchronizer<int, std::string, std::string> sync;

  sync.set_time_fcn<0>([](const int & i) { return i; });
  sync.set_time_fcn<1>([](const std::string & s) { return s.size(); });
  sync.set_time_fcn<2>([](const std::string & s) { return 2 * s.size(); });

  int sol_0;
  std::string sol_1;
  std::string sol_2;

  sync.register_callback([&sol_0, &sol_1, &sol_2](int && i, std::string && s1, std::string && s2) {
    sol_0 = i;
    sol_1 = s1;
    sol_2 = s2;
  });

  sync.add_and_search<0>(3);
  sync.add_and_search<0>(7);
  sync.add_and_search<0>(12);
  sync.add_and_search<1>("hello");
  sync.add_and_search<1>("merhaba");
  sync.add_and_search<1>("hello hello");
  sync.add_and_search<2>("hello");

  ASSERT_EQ(sol_0, 12);
  ASSERT_EQ(sol_1, "hello hello");
  ASSERT_EQ(sol_2, "hello");
}

TEST(SynchronizerTest, Example1)
{
  // example 1 from https://wiki.ros.org/message_filters/ApproximateTime
  cbr::Synchronizer<int, int, int, int> sync;

  sync.set_time_fcn<0>([](const int & i) { return i; });
  sync.set_time_fcn<1>([](const int & i) { return i; });
  sync.set_time_fcn<2>([](const int & i) { return i; });
  sync.set_time_fcn<3>([](const int & i) { return i; });

  int cb0 = 0, cb1 = 0, cb2 = 0, cb3 = 0;

  sync.register_callback([&cb0, &cb1, &cb2, &cb3](int && i0, int && i1, int && i2, int && i3) {
    cb0 = i0;
    cb1 = i1;
    cb2 = i2;
    cb3 = i3;
  });

  // first set
  sync.add_and_search<2>(10);
  sync.add_and_search<0>(11);
  sync.add_and_search<1>(12);
  sync.add_and_search<3>(13);  // pivot

  // second set
  sync.add_and_search<1>(20);
  sync.add_and_search<0>(21);
  ASSERT_EQ(cb0, 0);
  ASSERT_EQ(cb1, 0);
  ASSERT_EQ(cb2, 0);
  ASSERT_EQ(cb3, 0);
  sync.add_and_search<2>(22);  // trigger solution for set1
  ASSERT_EQ(cb2, 10);
  ASSERT_EQ(cb0, 11);
  ASSERT_EQ(cb1, 12);
  ASSERT_EQ(cb3, 13);
  sync.add_and_search<3>(23);  // pivot

  // dropped value
  sync.add_and_search<3>(26);

  // third set
  sync.add_and_search<0>(30);
  sync.add_and_search<1>(31);
  sync.add_and_search<3>(32);
  ASSERT_EQ(cb2, 10);
  ASSERT_EQ(cb0, 11);
  ASSERT_EQ(cb1, 12);
  ASSERT_EQ(cb3, 13);
  sync.add_and_search<2>(33);  // pivot, trigger solution for set2
  ASSERT_EQ(cb1, 20);
  ASSERT_EQ(cb0, 21);
  ASSERT_EQ(cb2, 22);
  ASSERT_EQ(cb3, 23);

  // dropped value
  sync.add_and_search<3>(34);

  // fourth set
  sync.add_and_search<3>(40);
  sync.add_and_search<2>(41);
  sync.add_and_search<1>(42);
  ASSERT_EQ(cb1, 20);
  ASSERT_EQ(cb0, 21);
  ASSERT_EQ(cb2, 22);
  ASSERT_EQ(cb3, 23);
  sync.add_and_search<0>(43);  // pivot, trigger solution for set3
  ASSERT_EQ(cb0, 30);
  ASSERT_EQ(cb1, 31);
  ASSERT_EQ(cb3, 32);
  ASSERT_EQ(cb2, 33);

  // fifth set
  sync.add_and_search<1>(46);
  sync.add_and_search<2>(46);
  ASSERT_EQ(cb0, 30);
  ASSERT_EQ(cb1, 31);
  ASSERT_EQ(cb3, 32);
  ASSERT_EQ(cb2, 33);
  sync.add_and_search<3>(47);  // trigger solution for set4
  ASSERT_EQ(cb3, 40);
  ASSERT_EQ(cb2, 41);
  ASSERT_EQ(cb1, 42);
  ASSERT_EQ(cb0, 43);
  sync.add_and_search<0>(47);  // pivot

  // sixth set
  sync.add_and_search<3>(60);
  sync.add_and_search<2>(65);
  sync.add_and_search<0>(67);
  ASSERT_EQ(cb3, 40);
  ASSERT_EQ(cb2, 41);
  ASSERT_EQ(cb1, 42);
  ASSERT_EQ(cb0, 43);
  sync.add_and_search<1>(70);  // pivot, trigger solution for set5
  ASSERT_EQ(cb1, 46);
  ASSERT_EQ(cb2, 46);
  ASSERT_EQ(cb3, 47);
  ASSERT_EQ(cb0, 47);

  // dropped value
  sync.add_and_search<2>(72);

  // seventh set
  sync.add_and_search<2>(75);
  sync.add_and_search<0>(79);
  sync.add_and_search<1>(80);
  ASSERT_EQ(cb1, 46);
  ASSERT_EQ(cb2, 46);
  ASSERT_EQ(cb3, 47);
  ASSERT_EQ(cb0, 47);
  sync.add_and_search<3>(82);  // pivot, trigger solution for set6
  ASSERT_EQ(cb3, 60);
  ASSERT_EQ(cb2, 65);
  ASSERT_EQ(cb0, 67);
  ASSERT_EQ(cb1, 70);

  // eigth set
  sync.add_and_search<1>(90);
  sync.add_and_search<2>(91);
  ASSERT_EQ(cb3, 60);
  ASSERT_EQ(cb2, 65);
  ASSERT_EQ(cb0, 67);
  ASSERT_EQ(cb1, 70);
  sync.add_and_search<0>(92);  // trigger solution for set7
  ASSERT_EQ(cb2, 75);
  ASSERT_EQ(cb0, 79);
  ASSERT_EQ(cb1, 80);
  ASSERT_EQ(cb3, 82);
  sync.add_and_search<3>(93);
}

TEST(SynchronizerTest, Example2)
{
  // example 2 from https://wiki.ros.org/message_filters/ApproximateTime
  cbr::Synchronizer<int, int> sync;

  sync.set_time_fcn<0>([](const int & i) { return i; });
  sync.set_time_fcn<1>([](const int & i) { return i; });

  std::vector<std::pair<int, int>> sets;

  sync.register_callback([&sets](int && i0, int && i1) { sets.push_back({i0, i1}); });

  // set 1
  for (int i = 0; i != 10; ++i) { sync.add_and_search<0>(2 + 50 * i); }

  // set 2
  for (int i = 0; i != 11; ++i) { sync.add_and_search<1>(40 * i); }

  ASSERT_EQ(sets[0].first, 2);
  ASSERT_EQ(sets[0].second, 0);

  ASSERT_EQ(sets[1].first, 52);
  ASSERT_EQ(sets[1].second, 40);

  ASSERT_EQ(sets[2].first, 102);
  ASSERT_EQ(sets[2].second, 120);

  ASSERT_EQ(sets[3].first, 152);
  ASSERT_EQ(sets[3].second, 160);

  ASSERT_EQ(sets[4].first, 202);
  ASSERT_EQ(sets[4].second, 200);

  ASSERT_EQ(sets[5].first, 252);
  ASSERT_EQ(sets[5].second, 240);

  ASSERT_EQ(sets[6].first, 302);
  ASSERT_EQ(sets[6].second, 320);

  ASSERT_EQ(sets[7].first, 352);
  ASSERT_EQ(sets[7].second, 360);
}

TEST(SynchronizerTest, UniquePtr)
{
  auto object1 = std::make_unique<int>(10);
  auto object2 = std::make_unique<int>(11);
  auto object3 = std::make_unique<int>(12);

  auto addr1 = reinterpret_cast<intptr_t>(object1.get());
  auto addr2 = reinterpret_cast<intptr_t>(object2.get());

  cbr::Synchronizer<std::unique_ptr<int>, std::unique_ptr<int>> sync;

  sync.set_time_fcn<0>([](const std::unique_ptr<int> & p) { return *p; });
  sync.set_time_fcn<1>([](const std::unique_ptr<int> & p) { return *p; });

  intptr_t res1, res2;
  sync.register_callback([&res1, &res2](std::unique_ptr<int> && p1, std::unique_ptr<int> && p2) {
    // can move out of container
    auto u1 = std::move(p1);
    auto u2 = std::move(p2);

    res1 = reinterpret_cast<intptr_t>(u1.get());
    res2 = reinterpret_cast<intptr_t>(u2.get());
  });

  // can move into container
  sync.add_and_search<0>(std::move(object1));
  sync.add_and_search<1>(std::move(object2));
  sync.add_and_search<0>(std::move(object3));  // trigger callback

  ASSERT_EQ(addr1, res1);
  ASSERT_EQ(addr2, res2);
}

TEST(SynchronizerTest, l_or_r_values)
{
  // insert rvalues
  {
    std::vector<int> v1{1, 2, 3};
    std::vector<int> v2{2, 3, 4};
    std::vector<int> v3{3, 4, 5};

    intptr_t addr1 = reinterpret_cast<intptr_t>(v1.data());
    intptr_t addr2 = reinterpret_cast<intptr_t>(v2.data());

    cbr::Synchronizer<std::vector<int>, std::vector<int>> sync;

    sync.set_time_fcn<0>([](const std::vector<int> & p) { return p[0]; });
    sync.set_time_fcn<1>([](const std::vector<int> & p) { return p[0]; });

    intptr_t res1, res2;
    sync.register_callback([&res1, &res2](std::vector<int> && p1, std::vector<int> && p2) {
      res1 = reinterpret_cast<intptr_t>(p1.data());
      res2 = reinterpret_cast<intptr_t>(p2.data());
    });

    // can move into container
    sync.add_and_search<0>(std::move(v1));
    sync.add_and_search<1>(std::move(v2));
    sync.add_and_search<0>(std::move(v3));  // trigger callback

    // expect internal data ptr of vectors to remain unchanged
    ASSERT_EQ(addr1, res1);
    ASSERT_EQ(addr2, res2);
  }

  // insert lvalues
  {
    const std::vector<int> v1{1, 2, 3};
    const std::vector<int> v2{2, 3, 4};
    const std::vector<int> v3{3, 4, 5};

    intptr_t addr1 = reinterpret_cast<intptr_t>(v1.data());
    intptr_t addr2 = reinterpret_cast<intptr_t>(v2.data());

    cbr::Synchronizer<std::vector<int>, std::vector<int>> sync;

    sync.set_time_fcn<0>([](const std::vector<int> & p) { return p[0]; });
    sync.set_time_fcn<1>([](const std::vector<int> & p) { return p[0]; });

    intptr_t res1, res2;
    sync.register_callback([&res1, &res2](std::vector<int> && p1, std::vector<int> && p2) {
      res1 = reinterpret_cast<intptr_t>(p1.data());
      res2 = reinterpret_cast<intptr_t>(p2.data());
    });

    // can move into container
    sync.add_and_search<0>(v1);
    sync.add_and_search<1>(v2);
    sync.add_and_search<0>(v3);  // trigger callback

    // expect address to have changed when data was copied into synchronizer
    ASSERT_NE(addr1, res1);
    ASSERT_NE(addr2, res2);
  }
}

TEST(SynchronizerTest, withDelta)
{
  cbr::Synchronizer<int, int, int, int> sync(15);

  sync.set_time_fcn<0>([](const int & i) { return i; });
  sync.set_time_fcn<1>([](const int & i) { return i; });
  sync.set_time_fcn<2>([](const int & i) { return i; });
  sync.set_time_fcn<3>([](const int & i) { return i; });

  int cb0 = 0, cb1 = 0, cb2 = 0, cb3 = 0;

  sync.register_callback([&cb0, &cb1, &cb2, &cb3](int && i0, int && i1, int && i2, int && i3) {
    cb0 = i0;
    cb1 = i1;
    cb2 = i2;
    cb3 = i3;
  });

  // first set
  sync.add_and_search<2>(10);
  sync.add_and_search<0>(11);
  sync.add_and_search<1>(12);
  sync.add_and_search<3>(13);  // pivot

  // second set: too close and skipped
  sync.add_and_search<1>(20);
  sync.add_and_search<0>(21);
  ASSERT_EQ(cb0, 0);
  ASSERT_EQ(cb1, 0);
  ASSERT_EQ(cb2, 0);
  ASSERT_EQ(cb3, 0);
  sync.add_and_search<2>(22);  // trigger solution for set1
  ASSERT_EQ(cb2, 10);
  ASSERT_EQ(cb0, 11);
  ASSERT_EQ(cb1, 12);
  ASSERT_EQ(cb3, 13);
  sync.add_and_search<3>(23);

  // third set
  sync.add_and_search<0>(30);
  sync.add_and_search<1>(31);
  sync.add_and_search<3>(32);
  sync.add_and_search<2>(33);  // pivot

  // dropped value
  sync.add_and_search<3>(34);

  // fourth set
  sync.add_and_search<3>(40);
  sync.add_and_search<2>(41);
  sync.add_and_search<1>(42);
  ASSERT_EQ(cb2, 10);
  ASSERT_EQ(cb0, 11);
  ASSERT_EQ(cb1, 12);
  ASSERT_EQ(cb3, 13);
  sync.add_and_search<0>(43);  // trigger solution for set 3
  ASSERT_EQ(cb0, 30);
  ASSERT_EQ(cb1, 31);
  ASSERT_EQ(cb3, 32);
  ASSERT_EQ(cb2, 33);
}

TEST(SynchronizerTest, fallOut)
{
  // example 1 from https://wiki.ros.org/message_filters/ApproximateTime
  cbr::Synchronizer<int, int, int, int> sync;

  sync.set_time_fcn<0>([](const int & i) { return i; });
  sync.set_time_fcn<1>([](const int & i) { return i; });
  sync.set_time_fcn<2>([](const int & i) { return i; });
  sync.set_time_fcn<3>([](const int & i) { return i; });

  int cb0 = 0, cb1 = 0, cb2 = 0, cb3 = 0;

  sync.register_callback([&cb0, &cb1, &cb2, &cb3](int && i0, int && i1, int && i2, int && i3) {
    cb0 = i0;
    cb1 = i1;
    cb2 = i2;
    cb3 = i3;
  });

  int cbf0 = -1, cbf1 = -1, cbf2 = -1, cbf3 = -1;

  sync.register_nonsync_callback<0>([&cbf0](int && i) { cbf0 = i; });

  sync.register_nonsync_callback<1>([&cbf1](int && i) { cbf1 = i; });

  sync.register_nonsync_callback<2>([&cbf2](int && i) { cbf2 = i; });

  sync.register_nonsync_callback<3>([&cbf3](int && i) { cbf3 = i; });

  // first set
  sync.add_and_search<2>(10);
  sync.add_and_search<0>(11);
  sync.add_and_search<1>(12);
  sync.add_and_search<3>(13);  // pivot

  // second set
  sync.add_and_search<1>(20);
  sync.add_and_search<0>(21);
  ASSERT_EQ(cb0, 0);
  ASSERT_EQ(cb1, 0);
  ASSERT_EQ(cb2, 0);
  ASSERT_EQ(cb3, 0);
  sync.add_and_search<2>(22);  // trigger solution for set1
  ASSERT_EQ(cb2, 10);
  ASSERT_EQ(cb0, 11);
  ASSERT_EQ(cb1, 12);
  ASSERT_EQ(cb3, 13);
  sync.add_and_search<3>(23);  // pivot

  // dropped value
  sync.add_and_search<3>(26);

  // third set
  sync.add_and_search<0>(30);
  sync.add_and_search<1>(31);
  sync.add_and_search<3>(32);  // triggers drop of missed value 26
  ASSERT_EQ(cb2, 10);
  ASSERT_EQ(cb0, 11);
  ASSERT_EQ(cb1, 12);
  ASSERT_EQ(cb3, 13);
  ASSERT_EQ(cbf3, -1);         // verify callback not yet called
  sync.add_and_search<2>(33);  // pivot, trigger solution for set2 and missed value
  ASSERT_EQ(cbf3, 26);         // veryfy callback was called on dropped value
  ASSERT_EQ(cb1, 20);
  ASSERT_EQ(cb0, 21);
  ASSERT_EQ(cb2, 22);
  ASSERT_EQ(cb3, 23);

  // dropped value
  sync.add_and_search<3>(34);

  // fourth set
  sync.add_and_search<3>(40);  // triggers drop value callback
  sync.add_and_search<2>(41);
  sync.add_and_search<1>(42);
  ASSERT_EQ(cb1, 20);
  ASSERT_EQ(cb0, 21);
  ASSERT_EQ(cb2, 22);
  ASSERT_EQ(cb3, 23);
  sync.add_and_search<0>(43);  // pivot, trigger solution for set3
  ASSERT_EQ(cbf3, 34);         // verify callback was called on dropped value 34
  ASSERT_EQ(cb0, 30);
  ASSERT_EQ(cb1, 31);
  ASSERT_EQ(cb3, 32);
  ASSERT_EQ(cb2, 33);

  // fifth set
  sync.add_and_search<1>(46);
  sync.add_and_search<2>(46);
  ASSERT_EQ(cb0, 30);
  ASSERT_EQ(cb1, 31);
  ASSERT_EQ(cb3, 32);
  ASSERT_EQ(cb2, 33);
  sync.add_and_search<3>(47);  // trigger solution for set4
  ASSERT_EQ(cb3, 40);
  ASSERT_EQ(cb2, 41);
  ASSERT_EQ(cb1, 42);
  ASSERT_EQ(cb0, 43);

  sync.add_and_search<0>(47);  // pivot

  // sixth set
  sync.add_and_search<3>(60);
  sync.add_and_search<2>(65);
  sync.add_and_search<0>(67);
  ASSERT_EQ(cb3, 40);
  ASSERT_EQ(cb2, 41);
  ASSERT_EQ(cb1, 42);
  ASSERT_EQ(cb0, 43);
  sync.add_and_search<1>(70);  // pivot, trigger solution for set5
  ASSERT_EQ(cb1, 46);
  ASSERT_EQ(cb2, 46);
  ASSERT_EQ(cb3, 47);
  ASSERT_EQ(cb0, 47);

  // dropped value
  sync.add_and_search<2>(72);

  // seventh set
  sync.add_and_search<2>(75);
  sync.add_and_search<0>(79);
  sync.add_and_search<1>(80);
  ASSERT_EQ(cb1, 46);
  ASSERT_EQ(cb2, 46);
  ASSERT_EQ(cb3, 47);
  ASSERT_EQ(cb0, 47);
  ASSERT_EQ(cbf2, -1);         // verify callback was called on dropped value 72
  sync.add_and_search<3>(82);  // pivot, trigger solution for set6
  ASSERT_EQ(cbf2, 72);         // verify callback was called on dropped value 72
  ASSERT_EQ(cb3, 60);
  ASSERT_EQ(cb2, 65);
  ASSERT_EQ(cb0, 67);
  ASSERT_EQ(cb1, 70);

  // eigth set
  sync.add_and_search<1>(90);
  sync.add_and_search<2>(91);
  ASSERT_EQ(cb3, 60);
  ASSERT_EQ(cb2, 65);
  ASSERT_EQ(cb0, 67);
  ASSERT_EQ(cb1, 70);
  sync.add_and_search<0>(92);  // trigger solution for set7
  ASSERT_EQ(cb2, 75);
  ASSERT_EQ(cb0, 79);
  ASSERT_EQ(cb1, 80);
  ASSERT_EQ(cb3, 82);
  sync.add_and_search<3>(93);

  ASSERT_EQ(cbf0, -1);  // verify callback was never called
  ASSERT_EQ(cbf1, -1);  // verify callback was never called
}

TEST(SynchronizerTest, fallOut2)
{
  // example 1 from https://wiki.ros.org/message_filters/ApproximateTime
  cbr::Synchronizer<int, int> sync;

  sync.set_time_fcn<0>([](const int & i) { return i; });
  sync.set_time_fcn<1>([](const int & i) { return i; });

  int cb0 = -1, cb1 = -1;

  sync.register_callback([&cb0, &cb1](int && i0, int && i1) {
    cb0 = i0;
    cb1 = i1;
  });

  std::vector<int> missed_0;

  sync.register_nonsync_callback<0>([&missed_0](int && i) { missed_0.push_back(i); });

  for (int i = 0; i < 10; i++) { sync.add_and_search<0>(i); }

  sync.add_and_search<1>(11);

  sync.add_and_search<0>(12);

  // 12 in queue 0 should be matched with 11 in queue 1
  ASSERT_EQ(cb0, 12);
  ASSERT_EQ(cb1, 11);

  // rest should be missed
  ASSERT_EQ(missed_0.size(), size_t(10));
  for (size_t i = 0; i < 10; i++) { ASSERT_EQ(missed_0[i], static_cast<int>(i)); }
}

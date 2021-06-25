// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#include <gtest/gtest.h>

#include <boost/hana/adapt_struct.hpp>

#include <string>

#include "cbr_utils/introspection.hpp"
#include "cbr_utils/static_for.hpp"
#include "cbr_utils/utils.hpp"

namespace cbr
{
struct HanaStruct
{
  BOOST_HANA_DEFINE_STRUCT(
    HanaStruct,
    (std::string, a),
    (int, b),
    (double, c)
  );
};
}  // namespace cbr

TEST(Introspection, to_tuple)
{
  cbr::HanaStruct fs1{"a1", 0, 1.};
  auto a1 = cbr::copy_to_tuple(fs1);
  static_assert(std::is_same_v<std::tuple_element_t<0, decltype(a1)>, std::string>);
  static_assert(std::is_same_v<std::tuple_element_t<1, decltype(a1)>, int>);
  static_assert(std::is_same_v<std::tuple_element_t<2, decltype(a1)>, double>);
  ASSERT_EQ(std::get<0>(a1), "a1");
  ASSERT_EQ(std::get<1>(a1), 0);
  ASSERT_EQ(std::get<2>(a1), 1.);

  auto b1 = cbr::bind_to_tuple<const cbr::HanaStruct>(fs1);
  static_assert(std::is_same_v<std::tuple_element_t<0, decltype(b1)>, const std::string &>);
  static_assert(std::is_same_v<std::tuple_element_t<1, decltype(b1)>, const int &>);
  static_assert(std::is_same_v<std::tuple_element_t<2, decltype(b1)>, const double &>);
  ASSERT_EQ(std::get<0>(b1), "a1");
  ASSERT_EQ(std::get<1>(b1), 0);
  ASSERT_EQ(std::get<2>(b1), 1.);

  fs1.a = "a2";
  fs1.b = 2;
  fs1.c = 3.;
  ASSERT_EQ(std::get<0>(b1), "a2");
  ASSERT_EQ(std::get<1>(b1), 2);
  ASSERT_EQ(std::get<2>(b1), 3.);

  cbr::HanaStruct fs2{"a1", 0, 1.};
  auto a2 = cbr::copy_to_tuple(fs2);
  static_assert(std::is_same_v<std::tuple_element_t<0, decltype(a2)>, std::string>);
  static_assert(std::is_same_v<std::tuple_element_t<1, decltype(a2)>, int>);
  static_assert(std::is_same_v<std::tuple_element_t<2, decltype(a2)>, double>);
  ASSERT_EQ(std::get<0>(a2), "a1");
  ASSERT_EQ(std::get<1>(a2), 0);
  ASSERT_EQ(std::get<2>(a2), 1.);

  auto b2 = cbr::bind_to_tuple(fs2);
  static_assert(std::is_same_v<std::tuple_element_t<0, decltype(b2)>, std::string &>);
  static_assert(std::is_same_v<std::tuple_element_t<1, decltype(b2)>, int &>);
  static_assert(std::is_same_v<std::tuple_element_t<2, decltype(b2)>, double &>);
  ASSERT_EQ(std::get<0>(b2), "a1");
  ASSERT_EQ(std::get<1>(b2), 0);
  ASSERT_EQ(std::get<2>(b2), 1.);

  fs2.a = "a2";
  fs2.b = 2;
  fs2.c = 3.;
  ASSERT_EQ(std::get<0>(b2), "a2");
  ASSERT_EQ(std::get<1>(b2), 2);
  ASSERT_EQ(std::get<2>(b2), 3.);

  std::get<0>(b2) = "a3";
  std::get<1>(b2) = 4;
  std::get<2>(b2) = 5.;

  ASSERT_EQ(fs2.a, "a3");
  ASSERT_EQ(fs2.b, 4);
  ASSERT_EQ(fs2.c, 5.);

  auto a3 = cbr::copy_to_tuple(cbr::HanaStruct{"a1", 0, 1.});
  static_assert(std::is_same_v<std::tuple_element_t<0, decltype(a3)>, std::string>);
  static_assert(std::is_same_v<std::tuple_element_t<1, decltype(a3)>, int>);
  static_assert(std::is_same_v<std::tuple_element_t<2, decltype(a3)>, double>);
  ASSERT_EQ(std::get<0>(a3), "a1");
  ASSERT_EQ(std::get<1>(a3), 0);
  ASSERT_EQ(std::get<2>(a3), 1.);
}

TEST(Introspection, StaticForAggregate)
{
  std::pair<double, int> p{1., 2};

  cbr::static_for_aggregate(
    p, [](auto v) {
      if constexpr (std::is_same_v<std::decay_t<decltype(v)>, double>) {
        ASSERT_EQ(v, 1.);
        v = 2.;
      } else {
        ASSERT_EQ(v, 2);
        v = 3;
      }
    });
  ASSERT_EQ(p.first, 1.);
  ASSERT_EQ(p.second, 2);

  cbr::static_for_aggregate(
    p, [](auto & v) {
      if constexpr (std::is_same_v<std::decay_t<decltype(v)>, double>) {
        ASSERT_EQ(v, 1.);
        v = 2.;
      } else {
        ASSERT_EQ(v, 2);
        v = 3;
      }
    });
  ASSERT_EQ(p.first, 2.);
  ASSERT_EQ(p.second, 3);

  cbr::static_for_aggregate(
    p, [](const auto & v) {
      if constexpr (std::is_same_v<std::decay_t<decltype(v)>, double>) {
        ASSERT_EQ(v, 2.);
      } else {
        ASSERT_EQ(v, 3);
      }
    });


  cbr::static_for_aggregate(
    std::make_pair(1., 2), [](auto v) {
      if constexpr (std::is_same_v<std::decay_t<decltype(v)>, double>) {
        ASSERT_EQ(v, 1.);
      } else {
        ASSERT_EQ(v, 2);
      }
    });


  cbr::static_for_aggregate(
    std::make_pair(1., 2), [](auto & v) {
      if constexpr (std::is_same_v<std::decay_t<decltype(v)>, double>) {
        ASSERT_EQ(v, 1.);
      } else {
        ASSERT_EQ(v, 2);
      }
    });

  cbr::static_for_aggregate(
    std::make_pair(1., 2), [](const auto & v) {
      if constexpr (std::is_same_v<std::decay_t<decltype(v)>, double>) {
        ASSERT_EQ(v, 1.);
      } else {
        ASSERT_EQ(v, 2);
      }
    });

  int i = 0;
  std::array a{1, 2, 3};
  cbr::static_for_aggregate(
    a, [&](auto & v) {
      i += v;
      v = 0;
    });
  ASSERT_EQ(i, 6);
  ASSERT_EQ(a[0], 0);
  ASSERT_EQ(a[1], 0);
  ASSERT_EQ(a[2], 0);

  i = 0;
  std::tuple t{1, 2, 3};
  cbr::static_for_aggregate(
    t, [&](auto & v) {
      i += v;
      v = 0;
    });
  ASSERT_EQ(i, 6);
  ASSERT_EQ(std::get<0>(t), 0);
  ASSERT_EQ(std::get<1>(t), 0);
  ASSERT_EQ(std::get<2>(t), 0);

  auto fun = cbr::overload
  {
    [&](const std::string & v) {
      i += v.size();
    },
    [&](int & v) {
      i += v;
      v = 0;
    },
    [&](double v) {
      i += 2 * static_cast<int>(v);
      v = 0.;
    }
  };

  i = 0;
  cbr::HanaStruct fs{"a1", 1, 2.};
  cbr::static_for_hana(fs, fun);
  ASSERT_EQ(i, 7);
  ASSERT_EQ(fs.b, 0);
  ASSERT_EQ(fs.c, 2.);

  i = 0;
  fs = cbr::HanaStruct {"a1", 1, 2.};
  cbr::static_for_hana(const_cast<const cbr::HanaStruct &>(fs), fun);
  ASSERT_EQ(i, 8);
  ASSERT_EQ(fs.b, 1);
  ASSERT_EQ(fs.c, 2.);

  i = 0;
  cbr::static_for_hana(cbr::HanaStruct{"a1", 1, 2.}, fun);
  ASSERT_EQ(i, 8);
}

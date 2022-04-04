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

#include "cbr_utils/digitset.hpp"
#include "cbr_utils/integer_pack.hpp"
#include "cbr_utils/integer_sequence.hpp"
#include "cbr_utils/static_for.hpp"
#include "cbr_utils/type_pack.hpp"
#include "cbr_utils/utils.hpp"

template<typename T>
struct TemplateExtractor
{
  static constexpr auto value = std::tuple_size_v<T>;
};

TEST(Utils, strcmpi)
{
  using cbr::strcmpi;
  ASSERT_TRUE(strcmpi("abcd", "abcd"));
  ASSERT_TRUE(strcmpi("abcd", "AbCd"));
  ASSERT_FALSE(strcmpi("abcd", "abcde"));
  ASSERT_FALSE(strcmpi(std::string("abcd"), "abcde"));
}

TEST(Utils, binary_perm)
{
  constexpr std::array<int, 3> min{-1, -2, -3};
  constexpr std::array<int, 3> max{3, 2, 1};
  constexpr auto vtx = cbr::binary_perm(min, max);

  ASSERT_EQ(vtx.size(), 2LU * 2LU * 2LU);
  ASSERT_EQ(vtx[0], (std::array<int, 3>{-1, -2, -3}));
  ASSERT_EQ(vtx[1], (std::array<int, 3>{3, -2, -3}));
  ASSERT_EQ(vtx[2], (std::array<int, 3>{-1, 2, -3}));
  ASSERT_EQ(vtx[3], (std::array<int, 3>{3, 2, -3}));
  ASSERT_EQ(vtx[4], (std::array<int, 3>{-1, -2, 1}));
  ASSERT_EQ(vtx[5], (std::array<int, 3>{3, -2, 1}));
  ASSERT_EQ(vtx[6], (std::array<int, 3>{-1, 2, 1}));
  ASSERT_EQ(vtx[7], (std::array<int, 3>{3, 2, 1}));

  constexpr auto vtxIdx = cbr::binary_perm<3>();
  for (std::size_t i = 0; i < vtxIdx.size(); i++) {
    for (std::size_t j = 0; j < 3; j++) { ASSERT_EQ(vtx[i][j], vtxIdx[i][j] ? max[j] : min[j]); }
  }

  constexpr auto vtx2 = cbr::binary_perm<3>(-3, 2);
  ASSERT_EQ(vtx2[0], (std::array<int, 3>{-3, -3, -3}));
  ASSERT_EQ(vtx2[1], (std::array<int, 3>{2, -3, -3}));
  ASSERT_EQ(vtx2[2], (std::array<int, 3>{-3, 2, -3}));
  ASSERT_EQ(vtx2[3], (std::array<int, 3>{2, 2, -3}));
  ASSERT_EQ(vtx2[4], (std::array<int, 3>{-3, -3, 2}));
  ASSERT_EQ(vtx2[5], (std::array<int, 3>{2, -3, 2}));
  ASSERT_EQ(vtx2[6], (std::array<int, 3>{-3, 2, 2}));
  ASSERT_EQ(vtx2[7], (std::array<int, 3>{2, 2, 2}));
}

TEST(Utils, digitset)
{
  constexpr cbr::digitset<2, 2> a0(0);
  static_assert(a0[0] == 0);
  static_assert(a0[1] == 0);
  static_assert(a0.count() == 0);
  static_assert(a0.to_ulong() == 0);

  constexpr cbr::digitset<2, 2> a1(1);
  static_assert(a1[0] == 1);
  static_assert(a1[1] == 0);
  static_assert(a1.count() == 1);
  static_assert(a1.to_ulong() == 1);

  constexpr cbr::digitset<2, 2> a2(2);
  static_assert(a2[0] == 0);
  static_assert(a2[1] == 1);
  static_assert(a2.count() == 1);
  static_assert(a2.to_ulong() == 2);

  constexpr cbr::digitset<2, 2> a3(3);
  static_assert(a3[0] == 1);
  static_assert(a3[1] == 1);
  static_assert(a3.count() == 2);
  static_assert(a3.to_ulong() == 3);

  constexpr cbr::digitset<2, 2> a4(4);
  static_assert(a4[0] == 0);
  static_assert(a4[1] == 0);

  constexpr cbr::digitset<2, 2> a5(5);
  static_assert(a5[0] == 1);
  static_assert(a5[1] == 0);

  cbr::digitset<2, 3> a(0);
  ASSERT_EQ(a[0], 0);
  ASSERT_EQ(a[1], 0);

  a = cbr::digitset<2, 3>(1);
  ASSERT_EQ(a[0], 1);
  ASSERT_EQ(a[1], 0);

  a = cbr::digitset<2, 3>(2);
  ASSERT_EQ(a[0], 2);
  ASSERT_EQ(a[1], 0);

  a = cbr::digitset<2, 3>(3);
  ASSERT_EQ(a[0], 0);
  ASSERT_EQ(a[1], 1);

  a = cbr::digitset<2, 3>(4);
  ASSERT_EQ(a[0], 1);
  ASSERT_EQ(a[1], 1);

  a = cbr::digitset<2, 3>(5);
  ASSERT_EQ(a[0], 2);
  ASSERT_EQ(a[1], 1);

  a = cbr::digitset<2, 3>(6);
  ASSERT_EQ(a[0], 0);
  ASSERT_EQ(a[1], 2);

  a = cbr::digitset<2, 3>(7);
  ASSERT_EQ(a[0], 1);
  ASSERT_EQ(a[1], 2);

  a = cbr::digitset<2, 3>(8);
  ASSERT_EQ(a[0], 2);
  ASSERT_EQ(a[1], 2);

  const auto s = cbr::detail::pow_fast<7>(4LU);
  for (std::size_t i = 0; i < s; i++) {
    cbr::digitset<7, 4> tmp(i);
    ASSERT_EQ(tmp.to_ulong(), i);
  }
}

TEST(Utils, all_unique)
{
  struct S
  {
    int a;
  };

  auto Scomp = [](const S & s1, const S & s2) { return s1.a == s2.a; };

  constexpr std::array a1{1, 2, 3, 4};
  constexpr std::array a2{1, 2, 3, 4, 3};
  constexpr std::array a3{S{1}, S{2}, S{3}, S{4}, S{3}};
  constexpr std::array a4{S{1}, S{2}, S{3}, S{4}, S{5}};

  constexpr bool b1 = cbr::all_unique(a1);
  constexpr bool b2 = cbr::all_unique(a2);
  constexpr bool b3 = cbr::all_unique(a3, Scomp);
  constexpr bool b4 = cbr::all_unique(a4, Scomp);

  static_assert(b1);
  static_assert(!b2);
  static_assert(!b3);
  static_assert(b4);
}

TEST(Utils, digit_perm)
{
  constexpr std::array<std::array<int, 3>, 2> vals{
    std::array<int, 3>{-1, 0, 1}, std::array<int, 3>{-2, 0, 3}};
  constexpr auto vtx = cbr::digit_perm(vals);

  ASSERT_EQ(vtx.size(), 3LU * 3LU);
  ASSERT_EQ(vtx[0], (std::array<int, 2>{-1, -2}));
  ASSERT_EQ(vtx[1], (std::array<int, 2>{0, -2}));
  ASSERT_EQ(vtx[2], (std::array<int, 2>{1, -2}));
  ASSERT_EQ(vtx[3], (std::array<int, 2>{-1, 0}));
  ASSERT_EQ(vtx[4], (std::array<int, 2>{0, 0}));
  ASSERT_EQ(vtx[5], (std::array<int, 2>{1, 0}));
  ASSERT_EQ(vtx[6], (std::array<int, 2>{-1, 3}));
  ASSERT_EQ(vtx[7], (std::array<int, 2>{0, 3}));
  ASSERT_EQ(vtx[8], (std::array<int, 2>{1, 3}));

  constexpr auto vtxIdx = cbr::digit_perm<2, 3>();
  for (std::size_t i = 0; i < vtxIdx.size(); i++) {
    for (std::size_t j = 0; j < 2; j++) { ASSERT_EQ(vtx[i][j], vals[j][vtxIdx[i][j]]); }
  }

  constexpr auto vtx2 = cbr::digit_perm<2>(std::array<int, 3>{3, 4, 5});
  ASSERT_EQ(vtx2[0], (std::array<int, 2>{3, 3}));
  ASSERT_EQ(vtx2[1], (std::array<int, 2>{4, 3}));
  ASSERT_EQ(vtx2[2], (std::array<int, 2>{5, 3}));
  ASSERT_EQ(vtx2[3], (std::array<int, 2>{3, 4}));
  ASSERT_EQ(vtx2[4], (std::array<int, 2>{4, 4}));
  ASSERT_EQ(vtx2[5], (std::array<int, 2>{5, 4}));
  ASSERT_EQ(vtx2[6], (std::array<int, 2>{3, 5}));
  ASSERT_EQ(vtx2[7], (std::array<int, 2>{4, 5}));
  ASSERT_EQ(vtx2[8], (std::array<int, 2>{5, 5}));
}

TEST(Utils, TypePack)
{
  using myPack = cbr::TypePack<bool, std::string, std::vector<int>>;
  static_assert(myPack::size == 3u);
  static_assert(std::is_same_v<myPack::tuple, std::tuple<bool, std::string, std::vector<int>>>);
  static_assert(std::is_same_v<myPack::tuple, myPack::unpack<std::tuple>>);
  static_assert(std::is_same_v<std::tuple<bool, std::string, std::vector<int>, double, int>,
    myPack::unpack<std::tuple, double, int>>);
  static_assert(std::is_same_v<std::tuple<double, int, bool, std::string, std::vector<int>>,
    myPack::unpack_prefixed<std::tuple, double, int>>);

  using nestedPack1 = myPack::apply<std::vector>;
  static_assert(nestedPack1::size == 3u);
  static_assert(std::is_same_v<nestedPack1::type<0>, std::vector<bool>>);
  static_assert(std::is_same_v<nestedPack1::type<1>, std::vector<std::string>>);
  static_assert(std::is_same_v<nestedPack1::type<2>, std::vector<std::vector<int>>>);

  using nestedPack2 = myPack::apply<std::tuple, int>;
  static_assert(nestedPack2::size == 3u);
  static_assert(std::is_same_v<nestedPack2::type<0>, std::tuple<bool, int>>);
  static_assert(std::is_same_v<nestedPack2::type<1>, std::tuple<std::string, int>>);
  static_assert(std::is_same_v<nestedPack2::type<2>, std::tuple<std::vector<int>, int>>);

  using nestedPack3 = myPack::apply_prefixed<std::tuple, int>;
  static_assert(nestedPack3::size == 3u);
  static_assert(std::is_same_v<nestedPack3::type<0>, std::tuple<int, bool>>);
  static_assert(std::is_same_v<nestedPack3::type<1>, std::tuple<int, std::string>>);
  static_assert(std::is_same_v<nestedPack3::type<2>, std::tuple<int, std::vector<int>>>);

  using joinedPack = typename cbr::typepack_cat<nestedPack1, nestedPack2, nestedPack3>::type;
  static_assert(std::is_same_v<joinedPack::type<0>, std::vector<bool>>);
  static_assert(std::is_same_v<joinedPack::type<3>, std::tuple<bool, int>>);
  static_assert(std::is_same_v<joinedPack::type<6>, std::tuple<int, bool>>);

  using myPack2 = cbr::TypePack<volatile int, const volatile double &, std::vector<int> &>;

  static_assert(std::is_same_v<myPack2::decay, cbr::TypePack<int, double, std::vector<int>>>);

  static_assert(
    std::is_same_v<cbr::TypePack<int, float, double>::head<3>, cbr::TypePack<int, float, double>>);

  static_assert(
    std::is_same_v<cbr::TypePack<int, float, double>::head<2>, cbr::TypePack<int, float>>);

  static_assert(
    std::is_same_v<cbr::TypePack<int, float, double>::tail<2>, cbr::TypePack<float, double>>);

  static_assert(
    std::is_same_v<cbr::TypePack<int, float, double>::reversed, cbr::TypePack<double, float, int>>);

  static_assert(
    std::is_same_v<typename cbr::TypePack<int, float, double>::subset<std::index_sequence<0, 2>>,
      cbr::TypePack<int, double>>);

  using S1 = std::tuple<int>;
  using S2 = std::tuple<int, int>;

  using myPack3 = cbr::TypePack<S1, S2, S1>;
  static_assert(
    std::is_same_v<myPack3::member_v<TemplateExtractor>, std::integer_sequence<size_t, 1, 2, 1>>);
  using packDuplicate  = typename myPack::duplicate<3>;
  using packDuplicate2 = cbr::typepack_cat<myPack, myPack, myPack>::type;

  static_assert(std::is_same_v<packDuplicate, packDuplicate2>);
}

template<std::size_t...>
struct A
{};

template<std::size_t, typename...>
struct B
{};

template<typename, std::size_t...>
struct C
{};

template<typename, typename, std::size_t...>
struct D
{};

TEST(Utils, IntegerPack)
{
  using myPack0 = cbr::IntegerPack<int>;
  static_assert(std::is_same_v<myPack0::type, int>);
  static_assert(myPack0::size == 0U);

  using myPack = cbr::IndexPack<0, 2, 1>;
  static_assert(myPack::size == 3U);
  static_assert(std::is_same_v<myPack::type, std::size_t>);
  static_assert(std::is_same_v<myPack::integer_sequence, std::index_sequence<0, 2, 1>>);
  static_assert(myPack::array[0] == 0LU && myPack::array[1] == 2LU && myPack::array[2] == 1LU);
  static_assert(myPack::tuple == std::make_tuple(0LU, 2LU, 1LU));
  static_assert(myPack::array[0] == myPack::value<0>() && myPack::array[1] == myPack::value<1>()
                && myPack::array[2] == myPack::value<2>());

  using nestedPack1 = myPack::apply<A>;
  static_assert(nestedPack1::size == 3u);
  static_assert(std::is_same_v<nestedPack1::type<0>, A<0>>);
  static_assert(std::is_same_v<nestedPack1::type<1>, A<2>>);
  static_assert(std::is_same_v<nestedPack1::type<2>, A<1>>);

  using nestedPack2 = myPack::apply<A, 5>;
  static_assert(nestedPack2::size == 3u);
  static_assert(std::is_same_v<nestedPack2::type<0>, A<0, 5>>);
  static_assert(std::is_same_v<nestedPack2::type<1>, A<2, 5>>);
  static_assert(std::is_same_v<nestedPack2::type<2>, A<1, 5>>);

  using nestedPack3 = myPack::apply_typed<B, float, double>;
  static_assert(nestedPack3::size == 3u);
  static_assert(std::is_same_v<nestedPack3::type<0>, B<0, float, double>>);
  static_assert(std::is_same_v<nestedPack3::type<1>, B<2, float, double>>);
  static_assert(std::is_same_v<nestedPack3::type<2>, B<1, float, double>>);

  using nestedPack4 = myPack::apply_prefixed<A, 5>;
  static_assert(nestedPack4::size == 3u);
  static_assert(std::is_same_v<nestedPack4::type<0>, A<5, 0>>);
  static_assert(std::is_same_v<nestedPack4::type<1>, A<5, 2>>);
  static_assert(std::is_same_v<nestedPack4::type<2>, A<5, 1>>);

  using nestedPack5 = myPack::apply_prefixed_typed<C, double>;
  static_assert(nestedPack5::size == 3u);
  static_assert(std::is_same_v<nestedPack5::type<0>, C<double, 0>>);
  static_assert(std::is_same_v<nestedPack5::type<1>, C<double, 2>>);
  static_assert(std::is_same_v<nestedPack5::type<2>, C<double, 1>>);

  using nestedPack6 = myPack::apply_prefixed_typed_2<D, double, float>;
  static_assert(nestedPack6::size == 3u);
  static_assert(std::is_same_v<nestedPack6::type<0>, D<double, float, 0>>);
  static_assert(std::is_same_v<nestedPack6::type<1>, D<double, float, 2>>);
  static_assert(std::is_same_v<nestedPack6::type<2>, D<double, float, 1>>);

  using joinedPack = typename cbr::integerpack_cat<cbr::IntegerPack<int64_t, -1, -2>,
    cbr::IntegerPack<int, 1, 2>>::type;
  static_assert(std::is_same_v<joinedPack::type, int64_t>);
  static_assert(joinedPack::value<0>() == -1L);
  static_assert(joinedPack::value<1>() == -2L);
  static_assert(joinedPack::value<2>() == 1L);
  static_assert(joinedPack::value<3>() == 2L);

  static_assert(std::is_same_v<myPack::unpack<A>, A<0, 2, 1>>);
  static_assert(std::is_same_v<myPack::unpack<A, 5, 6>, A<0, 2, 1, 5, 6>>);
  static_assert(std::is_same_v<myPack::unpack_prefixed<A, 5, 6>, A<5, 6, 0, 2, 1>>);
  static_assert(std::is_same_v<myPack::unpack_prefixed_typed<C, double>, C<double, 0, 2, 1>>);
  static_assert(
    std::is_same_v<myPack::unpack_prefixed_typed_2<D, int, float>, D<int, float, 0, 2, 1>>);

  static_assert(std::is_same_v<myPack::duplicate<3>, cbr::IndexPack<0, 2, 1, 0, 2, 1, 0, 2, 1>>);

  static_assert(std::is_same_v<myPack::append<7, 8>, cbr::IndexPack<0, 2, 1, 7, 8>>);
  static_assert(std::is_same_v<myPack::append_front<7, 8>, cbr::IndexPack<7, 8, 0, 2, 1>>);

  static_assert(std::is_same_v<myPack::subset<std::index_sequence<0, 0, 2, 1, 2>>,
    cbr::IndexPack<0, 0, 1, 2, 1>>);

  static_assert(std::is_same_v<myPack::reversed, cbr::IndexPack<1, 2, 0>>);

  static_assert(std::is_same_v<myPack::head<3>, myPack>);
  static_assert(std::is_same_v<myPack::head<2>, cbr::IndexPack<0, 2>>);
  static_assert(std::is_same_v<myPack::head<1>, cbr::IndexPack<0>>);

  static_assert(std::is_same_v<myPack::tail<3>, myPack>);
  static_assert(std::is_same_v<myPack::tail<2>, cbr::IndexPack<2, 1>>);
  static_assert(std::is_same_v<myPack::tail<1>, cbr::IndexPack<1>>);

  using Is  = std::integer_sequence<bool, false, true, false>;
  using Is2 = std::index_sequence<3, 2, 1>;
  static_assert(cbr::toIntegerPack<Is>::size == 3UL);
  static_assert(cbr::makeIntegerPack<Is>::size == 3UL);
  static_assert(std::is_same_v<cbr::toIntegerPack<Is>::type, bool>);
  static_assert(std::is_same_v<cbr::toIntegerPack<Is>::integer_sequence, Is>);
  static_assert(std::is_same_v<cbr::toIndexPack<Is2>::integer_sequence, Is2>);

  static_assert(std::is_same_v<cbr::makeIntegerPack<int, 4>, cbr::IntegerPack<int, 0, 1, 2, 3>>);
  static_assert(std::is_same_v<cbr::makeIndexPack<4>, cbr::IndexPack<0, 1, 2, 3>>);

  std::size_t count = 0;
  cbr::IndexPack<1, 2, 4>::loop([&count](auto i) { count += i; });
  ASSERT_EQ(count, 7);

  static_assert(
    std::is_same_v<cbr::make_integer_sequence<int, 3, 6>, std::integer_sequence<int, 3, 4, 5, 6>>);
  static_assert(std::is_same_v<cbr::make_index_sequence<3, 6>, std::index_sequence<3, 4, 5, 6>>);

  static_assert(std::is_same_v<cbr::makeIntegerPack<int, 3>, cbr::IntegerPack<int, 0, 1, 2>>);
  static_assert(std::is_same_v<cbr::makeIntegerPack<int, 3, 6>, cbr::IntegerPack<int, 3, 4, 5, 6>>);
  static_assert(std::is_same_v<cbr::makeIndexPack<3, 6>, cbr::IndexPack<3, 4, 5, 6>>);
  static_assert(std::is_same_v<cbr::makeIndexPack<3>, cbr::IndexPack<0, 1, 2>>);
}

TEST(Utils, SubTuple)
{
  std::tuple<int, double, float> t(1, 2, 3);

  auto subt = cbr::sub_tuple(t, std::index_sequence<0, 2>{});

  static_assert(std::is_same_v<decltype(subt), std::tuple<int, float>>);

  ASSERT_EQ(std::get<0>(subt), std::get<0>(t));
  ASSERT_EQ(std::get<1>(subt), std::get<2>(t));
}

struct Dummy
{
  std::pair<int, double> Function1(std::vector<int>, double) { return {0, 0}; }

  std::pair<int, double> Function2(std::vector<int>, const double &) const { return {0, 0}; }
};

TEST(Utils, FunctionSignature)
{
  static_assert(std::is_same_v<cbr::signature<decltype(&Dummy::Function1)>::return_type,
    std::pair<int, double>>);
  static_assert(std::is_same_v<cbr::signature<decltype(&Dummy::Function1)>::argument_type,
    cbr::TypePack<std::vector<int>, double>>);

  static_assert(std::is_same_v<cbr::signature<decltype(&Dummy::Function2)>::return_type,
    std::pair<int, double>>);
  static_assert(std::is_same_v<cbr::signature<decltype(&Dummy::Function2)>::argument_type,
    cbr::TypePack<std::vector<int>, const double &>>);
}

template<int... Idx>
struct Dummy2;

TEST(Utils, IseqUtils)
{
  using iseq = std::integer_sequence<int, 2, 3, 4>;
  using test = cbr::iseq_apply_t<iseq, Dummy2>;
  static_assert(std::is_same_v<test, Dummy2<2, 3, 4>>);

  using psum = cbr::iseq_psum_t<std::integer_sequence<int, 5, 6, 12>>;
  static_assert(std::is_same_v<psum, std::integer_sequence<int, 0, 5, 11>>);

  static_assert(cbr::iseq_sum_v<std::integer_sequence<int, 0, 5, 11>> == 16);

  static_assert(std::is_same_v<
    cbr::iseq_join_t<std::integer_sequence<int, 0, 1, 2>, std::integer_sequence<int, 3, 1, 3>>,
    std::integer_sequence<int, 0, 1, 2, 3, 1, 3>>);
}

TEST(Utils, StaticFor)
{
  std::tuple<double, int> t;

  auto l1 = [](double & d) { d = d + 1.; };

  auto l2 = [](int & i) { i = i + 2; };

  auto visitor = cbr::overload{l1, l2};

  cbr::static_for_index<2>([&](auto i) {
    using T              = typename std::tuple_element<i.value, decltype(t)>::type;
    std::get<i.value>(t) = T(0);
    visitor(std::get<i.value>(t));
  });

  ASSERT_DOUBLE_EQ(std::get<double>(t), 1.);
  ASSERT_EQ(std::get<int>(t), 2);

  int count = 0;
  cbr::static_for_index<0, 9>([&count]([[maybe_unused]] auto i) { count++; });
  ASSERT_EQ(count, 10);

  count   = 0;
  auto l3 = [&i_ = count](int i) mutable { i_ += i; };
  cbr::static_for<int, 4>{l3};
  ASSERT_EQ(count, 6);

  cbr::static_for<int, 20>([&count]([[maybe_unused]] auto i) {
    count++;
    return count < 15;
  });
  ASSERT_EQ(count, 15);

  count = 0;
  cbr::static_for<std::index_sequence<1, 2, 4>>(
    [&count]([[maybe_unused]] auto i) { count += static_cast<int>(i); });
  ASSERT_EQ(count, 7);

  count = 0;
  cbr::static_for<std::index_sequence<1, 2, 4>>([&count]([[maybe_unused]] auto i) {
    if constexpr (i > 2) { return false; }
    count += static_cast<int>(i);
    return true;
  });
  ASSERT_EQ(count, 3);
}

TEST(Utils, Specialization)
{
  using T1 = std::vector<double>;
  using T2 = std::array<double, 3>;
  using T3 = std::tuple<double, int, bool>;
  using T4 = std::shared_ptr<double>;
  using T5 = std::unique_ptr<double>;

  static_assert(cbr::is_specialization_v<T1, std::vector>);
  static_assert(cbr::is_specialization_v<T3, std::tuple>);
  static_assert(cbr::is_specialization_v<T3, std::tuple>);
  static_assert(cbr::is_specialization_v<T4, std::shared_ptr>);
  static_assert(cbr::is_specialization_v<T5, std::unique_ptr>);

  static_assert(cbr::is_std_vector_v<T1> && !cbr::is_std_vector_v<T2> && !cbr::is_std_vector_v<T3>);

  static_assert(!cbr::is_std_array_v<T1> && cbr::is_std_array_v<T2> && !cbr::is_std_array_v<T3>);

  static_assert(!cbr::is_std_tuple_v<T1> && !cbr::is_std_tuple_v<T2> && cbr::is_std_tuple_v<T3>);
}

TEST(Utils, dateStr)
{
  const auto now     = std::chrono::system_clock::now();
  const auto nowStr0 = cbr::dateStr();

  const auto nowStr         = cbr::dateStr(now);
  const auto nowStrFullPres = cbr::dateStr(now, true);

  const auto now2         = cbr::fromDateStr(nowStr);
  const auto nowFullPres2 = cbr::fromDateStr(nowStrFullPres);

  const auto nowStr2         = cbr::dateStr(now2);
  const auto nowStrFullPres2 = cbr::dateStr(nowFullPres2, true);

  ASSERT_EQ(nowStr, nowStr0);
  ASSERT_EQ(nowStr, nowStr2);
  ASSERT_EQ(nowStrFullPres.substr(0, nowStrFullPres.length() - 2),
    nowStrFullPres2.substr(0, nowStrFullPres.length() - 2));
}

TEST(Utils, is_sorted)
{
  const std::vector<double> v01;
  const std::vector v02{1};
  const std::vector v1{1, 2, 3};
  const std::vector v2{1, 2, 3, 3};
  const std::vector v3{2, 1, 3};
  const std::vector v4{3, 3, 2};

  EXPECT_TRUE(cbr::is_sorted(v01.cbegin(), v01.cend()));
  EXPECT_TRUE(cbr::is_sorted(v02.cbegin(), v02.cend()));
  EXPECT_TRUE(cbr::is_sorted(v1.cbegin(), v1.cend()));
  EXPECT_TRUE(cbr::is_sorted(v2.cbegin(), v2.cend()));
  EXPECT_FALSE(cbr::is_strictly_sorted(v2.cbegin(), v2.cend()));
  EXPECT_FALSE(cbr::is_sorted(v3.cbegin(), v3.cend()));
  EXPECT_FALSE(cbr::is_sorted(v4.cbegin(), v4.cend()));
  EXPECT_FALSE(cbr::is_strictly_sorted(v4.cbegin(), v4.cend()));
}

TEST(Utils, ValidFilename)
{
  EXPECT_FALSE(cbr::isValidFilename("\0test", false));
  EXPECT_TRUE(cbr::isValidFilename("\1test", false));
  EXPECT_FALSE(cbr::isValidFilename("\1test"));
  EXPECT_TRUE(cbr::isValidFilename("test"));
  EXPECT_FALSE(cbr::isValidFilename("\\test"));
  EXPECT_TRUE(cbr::isValidFilename("\\test", false));
  EXPECT_FALSE(cbr::isValidFilename("/test"));
  EXPECT_FALSE(cbr::isValidFilename(":test"));
  EXPECT_FALSE(cbr::isValidFilename("*test"));
  EXPECT_FALSE(cbr::isValidFilename("?test"));
  EXPECT_FALSE(cbr::isValidFilename("\"test"));
  EXPECT_FALSE(cbr::isValidFilename("<test"));
  EXPECT_FALSE(cbr::isValidFilename(">test"));
  EXPECT_FALSE(cbr::isValidFilename("|test"));
  EXPECT_FALSE(cbr::isValidFilename("test "));
  EXPECT_FALSE(cbr::isValidFilename("test."));
}

TEST(Utils, formatDuration)
{
  EXPECT_DOUBLE_EQ(cbr::formatDuration(90.).first, 1.5);
  EXPECT_EQ(cbr::formatDuration(90.).second, "min");

  EXPECT_DOUBLE_EQ(cbr::formatDuration(5.).first, 5.);
  EXPECT_EQ(cbr::formatDuration(5.).second, "s");

  EXPECT_DOUBLE_EQ(cbr::formatDuration(1.).first, 1.);
  EXPECT_EQ(cbr::formatDuration(1.).second, "s");

  EXPECT_DOUBLE_EQ(cbr::formatDuration(0.1).first, 100.);
  EXPECT_EQ(cbr::formatDuration(0.1).second, "ms");

  EXPECT_DOUBLE_EQ(cbr::formatDuration(0.01).first, 10.);
  EXPECT_EQ(cbr::formatDuration(0.01).second, "ms");

  EXPECT_DOUBLE_EQ(cbr::formatDuration(0.001).first, 1.);
  EXPECT_EQ(cbr::formatDuration(0.001).second, "ms");

  EXPECT_DOUBLE_EQ(cbr::formatDuration(0.0001).first, 100.);
  EXPECT_EQ(cbr::formatDuration(0.0001).second, "us");

  EXPECT_DOUBLE_EQ(cbr::formatDuration(0.00001).first, 10.);
  EXPECT_EQ(cbr::formatDuration(0.00001).second, "us");

  EXPECT_DOUBLE_EQ(cbr::formatDuration(0.000001).first, 1.);
  EXPECT_EQ(cbr::formatDuration(0.000001).second, "us");

  EXPECT_DOUBLE_EQ(cbr::formatDuration(0.0000001).first, 0.1);
  EXPECT_EQ(cbr::formatDuration(0.0000001).second, "us");
}

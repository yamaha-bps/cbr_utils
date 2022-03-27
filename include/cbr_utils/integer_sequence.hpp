// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__INTEGER_SEQUENCE_HPP_
#define CBR_UTILS__INTEGER_SEQUENCE_HPP_

#include <tuple>
#include <type_traits>
#include <utility>

namespace cbr {

/***************************************************************************
 * \brief Build integer sequence between 2 numbers
 ***************************************************************************/
namespace detail {

template<typename T, T...>
struct make_iseq_impl_;

template<typename T>
struct make_iseq_impl_<T>
{
  using type = std::integer_sequence<T>;
};

template<typename T, T Val>
struct make_iseq_impl_<T, Val>
{
  constexpr static bool no_size = Val < T(1);
  constexpr static T ValNm1     = []() {
    if constexpr (Val < T(1)) {
      return 0;
    } else {
      return Val - T(1);
    }
  }();

  using type = std::
    conditional_t<no_size, std::integer_sequence<T>, typename make_iseq_impl_<T, 0, ValNm1>::type>;
};

template<typename T, T First, T... Ts>
struct make_iseq_impl_<T, First, First, Ts...>
{
  using type = std::integer_sequence<T, First, Ts...>;
};

template<typename T, T First, T Curr, T... Ts>
struct make_iseq_impl_<T, First, Curr, Ts...>
{
  using dir_t                = std::make_signed_t<T>;
  constexpr static dir_t dir = []() {
    if constexpr (First <= Curr) {
      return dir_t(-1);
    } else {
      return dir_t(1);
    }
  }();

  using type = typename make_iseq_impl_<T,
    First,
    static_cast<T>(static_cast<dir_t>(Curr) + dir),
    Curr,
    Ts...>::type;
};

}  // namespace detail

template<typename T, T... Vs>
using make_integer_sequence = typename detail::make_iseq_impl_<T, Vs...>::type;

template<std::size_t... Vs>
using make_index_sequence = typename detail::make_iseq_impl_<std::size_t, Vs...>::type;

/***************************************************************************
 * \brief std::integer_sequence utilities
 ***************************************************************************/

// Create sub-tuple using index_sequence, works on any tuple-like type that supports
// std::tuple_element_t and std::get
template<template<typename...> typename T, typename... S, std::size_t... I>
auto sub_tuple(const T<S...> & t, std::index_sequence<I...>)
{
  return T<std::tuple_element_t<I, T<S...>>...>(std::get<I>(t)...);
}

// Apply an integer_sequence<Int, I1, I2 ...> to a templated type T<Int...> to form T<I1, I2, ...>
template<typename ISeq, template<typename ISeq::value_type...> typename T>
struct iseq_apply;

template<typename Int, template<Int...> typename T, Int... I>
struct iseq_apply<std::integer_sequence<Int, I...>, T>
{
  using type = T<I...>;
};

template<typename ISeq, template<typename ISeq::value_type...> typename T>
using iseq_apply_t = typename iseq_apply<ISeq, T>::type;

// Cumulative prefix sum for std::integer_sequence (c.f. std::exclusive_scan)
template<typename Cur, typename ISeq, typename ISeq::value_type Sum>
struct iseq_psum
{
  using type = Cur;
};

template<typename T, T... Cur, T First, T... Rem, T Sum>
struct iseq_psum<std::integer_sequence<T, Cur...>, std::integer_sequence<T, First, Rem...>, Sum>
    : iseq_psum<std::integer_sequence<T, Cur..., Sum>,
        std::integer_sequence<T, Rem...>,
        Sum + First>
{};

template<class ISeq>
using iseq_psum_t =
  typename iseq_psum<std::integer_sequence<typename ISeq::value_type>, ISeq, 0>::type;

// Sum for std::integer_sequence
template<typename ISeq>
struct iseq_sum;

template<typename T, T... I>
struct iseq_sum<std::integer_sequence<T, I...>>
{
  static constexpr T value = (I + ...);
};

template<typename ISeq>
constexpr typename ISeq::value_type iseq_sum_v = iseq_sum<ISeq>::value;

// Join multiple std::integer_sequence's
template<typename...>
struct iseq_join;

template<typename Int, Int... I>
struct iseq_join<std::integer_sequence<Int, I...>>
{
  using type = std::integer_sequence<Int, I...>;
};

template<typename Int, Int... I1, Int... I2, typename... Rem>
struct iseq_join<std::integer_sequence<Int, I1...>, std::integer_sequence<Int, I2...>, Rem...>
{
  using type = typename iseq_join<std::integer_sequence<Int, I1..., I2...>, Rem...>::type;
};

template<typename... T>
using iseq_join_t = typename iseq_join<T...>::type;

}  // namespace cbr

#endif  // CBR_UTILS__INTEGER_SEQUENCE_HPP_

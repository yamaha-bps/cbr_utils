// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

/** @file */

#ifndef CBR_UTILS__INTEGER_SEQUENCE_HPP_
#define CBR_UTILS__INTEGER_SEQUENCE_HPP_

#include <tuple>
#include <type_traits>
#include <utility>

namespace cbr {

/// @cond

namespace detail {

// Utility class to build integer sequence
template<typename T, T...>
struct make_iseq_impl_;

// Utility class to build an empty integer sequence
template<typename T>
struct make_iseq_impl_<T>
{
  using type = std::integer_sequence<T>;
};

// Utility class to build an empty integer from 0 to Val excluded
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

// Utility class to build an empty integer from First to Ts included
template<typename T, T First, T... Ts>
struct make_iseq_impl_<T, First, First, Ts...>
{
  using type = std::integer_sequence<T, First, Ts...>;
};

// Utility class to build an empty integer from First to Ts included
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

/// @endcond

/**
 * @brief Create an std::integer_sequence.
 * @details - If Vs is 2 elements, create an integer sequence of all values between this 2 elements
 * (2 elements included).
 * - If Vs is 1 element, create an integer sequence from 0 to Vs (Vs excluded).
 *
 * Example:
 * ```
 * ```
 * @tparam T Type of the elements of the integer sequence.
 * @tparam Vs Construction parameters for the sequence.
 */
template<typename T, T... Vs>
using make_integer_sequence = typename detail::make_iseq_impl_<T, Vs...>::type;

/**
 * @brief Create an std::index_sequence.
 * @details - If Vs is 2 elements, create an index sequence of all values between this 2 elements
 * (2 elements included).
 * - If Vs is 1 element, create an index sequence from 0 to Vs (Vs excluded).
 *
 * @tparam Vs Construction parameters for the sequence.
 */
template<std::size_t... Vs>
using make_index_sequence = typename detail::make_iseq_impl_<std::size_t, Vs...>::type;

/**
 * @brief Extract sub-tuple from tuple-like object.
 * @details Works with any object that supports std::tuple_element_t and
 * std::get.
 *
 * @tparam T Type of the tuple-like object.
 * @tparam S Types of the elements of the tuple-like object.
 * @tparam I Indexes of the elements to extract.
 * @param t Tuple-like object.
 * @param iseq Integer sequence of indexes of the elements to extract.
 * @return Sub-tuple object.
 */
template<template<typename...> typename T, typename... S, std::size_t... I>
auto sub_tuple(const T<S...> & t, [[maybe_unused]] std::index_sequence<I...> iseq)
{
  return T<std::tuple_element_t<I, T<S...>>...>{std::get<I>(t)...};
}

/// @cond

// Apply an integer_sequence<Int, I1, I2 ...> to a templated type T<Int...> to form T<I1, I2, ...>
template<typename ISeq, template<typename ISeq::value_type...> typename T>
struct iseq_apply;

// Apply an integer_sequence<Int, I1, I2 ...> to a templated type T<Int...> to form T<I1, I2, ...>
template<typename Int, template<Int...> typename T, Int... I>
struct iseq_apply<std::integer_sequence<Int, I...>, T>
{
  using type = T<I...>;
};

/// @endcond

/**
 * @brief Application of a given templated type over each element of the sequence.
 * @details For example:
 * ```
 * template<int i, int j, int k>
 * struct S{};
 *
 * using T = std::integer_sequence<int, 0, 1, 2>;
 * using T2 = iseq_apply_t<T,S>;
 * ```
 *
 * Then T2 == S<0,1,2>.
 *
 * @tparam ISeq Integer sequence to apply templated type to.
 * @tparam T Templated type to use for application.
 */
template<typename ISeq, template<typename ISeq::value_type...> typename T>
using iseq_apply_t = typename iseq_apply<ISeq, T>::type;

/// @cond

// Exclusive prefix sum for std::integer_sequence
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

/// @endcond

/**
 * @brief Create an integer sequence of the prefix cumulative sums for an std::integer_sequence.
 * @details See std::exclusive_scan for more info.
 * Returns an std::integer_sequence accumulating the elements of the input sequence starting with 0.
 *
 * @tparam ISeq Integer sequence to accumulate.
 */
template<class ISeq>
using iseq_psum_t =
  typename iseq_psum<std::integer_sequence<typename ISeq::value_type>, ISeq, 0>::type;

/// @cond

// Sum of elements of an std::integer_sequence
template<typename ISeq>
struct iseq_sum;

// Sum of elements of an std::integer_sequence
template<typename T, T... I>
struct iseq_sum<std::integer_sequence<T, I...>>
{
  static constexpr T value = (I + ...);
};

/// @endcond

/**
 * @brief Sum of elements of an std::integer_sequence.
 * @details See std::accumulate for more info.
 *
 * @tparam ISeq Integer sequence to accumulate.
 */
template<typename ISeq>
constexpr typename ISeq::value_type iseq_sum_v = iseq_sum<ISeq>::value;

/// @cond

// Join multiple std::integer_sequence's
template<typename...>
struct iseq_join;

// Join multiple std::integer_sequence's
template<typename Int, Int... I>
struct iseq_join<std::integer_sequence<Int, I...>>
{
  using type = std::integer_sequence<Int, I...>;
};

// Join multiple std::integer_sequence's
template<typename Int, Int... I1, Int... I2, typename... Rem>
struct iseq_join<std::integer_sequence<Int, I1...>, std::integer_sequence<Int, I2...>, Rem...>
{
  using type = typename iseq_join<std::integer_sequence<Int, I1..., I2...>, Rem...>::type;
};

/// @endcond

/**
 * @brief Join multiple std::integer_sequence's
 *
 * @tparam T Integer sequences
 */
template<typename... T>
using iseq_join_t = typename iseq_join<T...>::type;

}  // namespace cbr

#endif  // CBR_UTILS__INTEGER_SEQUENCE_HPP_

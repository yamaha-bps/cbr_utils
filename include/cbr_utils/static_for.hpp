// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

/** @file */

#ifndef CBR_UTILS__STATIC_FOR_HPP_
#define CBR_UTILS__STATIC_FOR_HPP_

#include <experimental/type_traits>
#include <functional>
#include <type_traits>
#include <utility>

#include "integer_sequence.hpp"
#if __has_include(<boost/hana/version.hpp>)
#include "introspection.hpp"
#endif
#include "type_traits.hpp"

namespace cbr {

/// @cond

namespace detail {

// Static for loop implementation from using std::integer_sequence.
template<typename Lambda, typename T, T... Is>
void static_for_iseq_impl(Lambda && f, std::integer_sequence<T, Is...>)
{
  constexpr bool has_common_type = std::experimental::is_detected_v<std::common_type_t,
    std::invoke_result_t<Lambda, std::integral_constant<T, Is>>...>;

  static_assert(has_common_type,
    "Callable passed to the static_for function must all have the same return type.");

  if constexpr (has_common_type) {
    using return_t =
      std::common_type_t<std::invoke_result_t<Lambda, std::integral_constant<T, Is>>...>;

    if constexpr (std::is_same_v<return_t, bool>) {
      (std::invoke(f, std::integral_constant<T, Is>()) && ...);
    } else {
      (std::invoke(f, std::integral_constant<T, Is>()), ...);
    }
  }
}

// Static for loop implementation.
template<typename T, T... Vs>
struct static_for_impl;

// Static for loop implementation.
template<typename Is>
struct static_for_impl<Is>
{
  template<typename Lambda>
  explicit static_for_impl(Lambda && f)
  {
    if constexpr (Is::size() > typename Is::value_type(0)) {
      static_for_iseq_impl(std::forward<Lambda>(f), Is{});
    }
  }
};

// Static for loop implementation.
template<typename T, T First, T Last>
struct static_for_impl<T, First, Last>
{
  template<typename Lambda>
  explicit static_for_impl(Lambda && f)
  {
    static_for_iseq_impl(std::forward<Lambda>(f), make_integer_sequence<T, First, Last>{});
  }
};

// Static for loop implementation.
template<typename T, T N>
struct static_for_impl<T, N>
{
  template<typename Lambda>
  explicit static_for_impl(Lambda && f)
  {
    if constexpr (N > T(0)) {
      static_for_iseq_impl(std::forward<Lambda>(f), make_integer_sequence<T, N>{});
    }
  }
};

}  // namespace detail

/// @endcond

/**
 * @brief Compile time loop over integers.
 * @details Defines a type whose contructor takes a callable.
 * What is being passed to the callable are std::integral_contant objects whose values are
 * specified by the template parameters:
 * - If Vs is empty, then iterates the std::integer_sequence T.
 * - If Vs is 2 elements, iterates over all values between this 2 elements (2 elements included).
 * - If Vs is 1 element, iterates from 0 to Vs (Vs excluded).
 *
 * Example:
 * ```
 * template<int i>
 * struct S{
 *  void print(){
 *    std::cout << i << std::endl;
 *  }
 * };
 * static_for<int,2,3>([]{const auto i}{
 *    S<i.value> s;
 *    s.print();
 * });
 * ```
 * Notes:
 * - The callables passed to the static_for function must all have the same return type.
 * - If return type is bool, only iterates until callable returns false;
 *
 * @tparam T Type of the elements to iterate over, or std::integer_sequence
 * @tparam Vs Loop values parameters.
 */
template<typename T, T... Vs>
using static_for = detail::static_for_impl<T, Vs...>;

/**
 * @brief Same as static_for but with integer type set to std::size_t.
 */
template<std::size_t... Vs>
using static_for_index = detail::static_for_impl<std::size_t, Vs...>;

/// @cond
namespace detail {

template<typename Seq, typename Lambda, std::size_t... Is>
void static_for_aggregate_impl(Seq && s, Lambda && f, std::index_sequence<Is...>)
{
  using return_t = std::common_type_t<std::invoke_result_t<Lambda, decltype(std::get<Is>(s))>...>;
  if constexpr (std::is_same_v<return_t, bool>) {
    (std::invoke(f, std::get<Is>(s)) && ...);
  } else {
    (std::invoke(f, std::get<Is>(s)), ...);
  }
}

}  // namespace detail
/// @endcond

/**
 * @brief Compile time loop over fields of an aggregage.
 * @details Works for every type that supports std::get().
 * What is being passed to the callable are references (possibly const) to the fields of the
 * aggregage.

 * Example:
 * ```
 * std::tuple<int,double,float> t{0,1.,2.};
 * static_for_aggregate(t, []{const auto & v}{
 *   std::cout << v << std::endl;
 * });
 * ```
 * @tparam Seq Type of the aggregate object.
 * @tparam T Type of the callable.
 */
template<typename Seq, typename T>
void static_for_aggregate(Seq && s, T && f)
{
  using Seq_t = std::decay_t<Seq>;

  detail::static_for_aggregate_impl(
    std::forward<Seq>(s), std::forward<T>(f), std::make_index_sequence<std::tuple_size_v<Seq_t>>{});
}

#if __has_include(<boost/hana/version.hpp>)
/**
 * @brief Compile time loop over fields of a boost::hana::Struct.
 * @details Works the same as static_for_aggregate, but uses boost::hana::Struct machinery to
 * reference the fields.
 */
template<typename Seq, typename T>
void static_for_hana(Seq && s, T && f)
{
  using s_t = decltype(s);

  if constexpr (std::is_rvalue_reference_v<s_t>) {
    const auto t = copy_to_tuple(std::forward<Seq>(s));
    using N      = std::tuple_size<decltype(t)>;
    detail::static_for_aggregate_impl(t, std::forward<T>(f), std::make_index_sequence<N::value>{});
  } else {
    auto t  = bind_to_tuple(s);
    using N = std::tuple_size<decltype(t)>;
    detail::static_for_aggregate_impl(
      std::move(t), std::forward<T>(f), std::make_index_sequence<N::value>{});
  }
}
#endif

}  // namespace cbr

#endif  // CBR_UTILS__STATIC_FOR_HPP_

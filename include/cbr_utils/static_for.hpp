// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_ros/blob/master/LICENSE

#ifndef CBR_UTILS__STATIC_FOR_HPP_
#define CBR_UTILS__STATIC_FOR_HPP_

#include <experimental/type_traits>
#include <functional>
#include <type_traits>
#include <utility>

#include "type_traits.hpp"
#include "integer_sequence.hpp"
#include "introspection.hpp"

namespace cbr
{
/***************************************************************************
 * \brief Static for loop over integral_constant
 ***************************************************************************/
namespace detail
{

template<typename Lambda, typename T, T ... Is>
void static_for_iseq_impl(Lambda && f, std::integer_sequence<T, Is...>)
{
  constexpr bool has_common_type = std::experimental::is_detected_v<std::common_type_t,
      std::invoke_result_t<Lambda, std::integral_constant<T, Is>>...
  >;

  static_assert(
    has_common_type,
    "Operators passed to the static_for function must all have the same return type.");

  if constexpr (has_common_type) {
    using return_t = std::common_type_t<
      std::invoke_result_t<Lambda, std::integral_constant<T, Is>>...
    >;

    if constexpr (std::is_same_v<return_t, bool>) {
      (std::invoke(f, std::integral_constant<T, Is>()) && ...);
    } else {
      (std::invoke(f, std::integral_constant<T, Is>()), ...);
    }
  }
}

template<typename T, T... Vs>
struct static_for_impl;

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

template<typename T, T First, T Last>
struct static_for_impl<T, First, Last>
{
  template<typename Lambda>
  explicit static_for_impl(Lambda && f)
  {
    static_for_iseq_impl(std::forward<Lambda>(f), make_integer_sequence<T, First, Last>{});
  }
};

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

template<typename T, T... Vs>
using static_for = detail::static_for_impl<T, Vs...>;

template<std::size_t... Vs>
using static_for_index = detail::static_for_impl<std::size_t, Vs...>;

/***************************************************************************
 * \brief Static for loop over aggregate
 ***************************************************************************/
namespace detail
{

template<typename Seq, typename Lambda, std::size_t ... Is>
void static_for_aggregate_impl(Seq && s, Lambda && f, std::index_sequence<Is...>)
{
  using return_t = std::common_type_t<
    std::invoke_result_t<Lambda, decltype(std::get<Is>(s))>...
  >;
  if constexpr (std::is_same_v<return_t, bool>) {
    (std::invoke(f, std::get<Is>(s)) && ...);
  } else {
    (std::invoke(f, std::get<Is>(s)), ...);
  }
}

}  // namespace detail

template<typename Seq, typename Lambda>
void static_for_aggregate(Seq && s, Lambda && f)
{
  using Seq_t = std::decay_t<Seq>;
  using s_t = decltype(s);

  if constexpr (boost::hana::Struct<Seq_t>::value) {
    if constexpr (std::is_rvalue_reference_v<s_t>) {
      const auto t = copy_to_tuple(std::forward<Seq>(s));
      using N = std::tuple_size<decltype(t)>;
      detail::static_for_aggregate_impl(
        t,
        std::forward<Lambda>(f),
        std::make_index_sequence<N::value>{});
    } else {
      auto t = bind_to_tuple(s);
      using N = std::tuple_size<decltype(t)>;
      detail::static_for_aggregate_impl(
        std::move(t),
        std::forward<Lambda>(f),
        std::make_index_sequence<N::value>{});
    }
  } else {
    detail::static_for_aggregate_impl(
      std::forward<Seq>(s),
      std::forward<Lambda>(f),
      std::make_index_sequence<std::tuple_size_v<Seq_t>>{});
  }
}

}  // namespace cbr

#endif  // CBR_UTILS__STATIC_FOR_HPP_

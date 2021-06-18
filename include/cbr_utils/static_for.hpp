// Copyright 2020 Yamaha Motor Corporation, USA
#ifndef CBR_UTILS__STATIC_FOR_HPP_
#define CBR_UTILS__STATIC_FOR_HPP_

#include <type_traits>
#include <utility>

#include "type_traits.hpp"
#include "integer_sequence.hpp"

namespace cbr
{
/***************************************************************************
 * \brief Static for loop over integral_constant
 ***************************************************************************/
namespace detail
{

template<typename Lambda, typename T, T I, T ... Is>
void static_for_iseq_impl(Lambda && f, std::integer_sequence<T, I, Is...>)
{
  using Is_t = std::integer_sequence<T, Is...>;
  using I_t = std::integral_constant<T, I>;
  using return_t = std::invoke_result_t<Lambda, I_t>;
  if constexpr (std::is_same_v<return_t, bool>) {
    if (f(I_t{})) {
      if constexpr (sizeof...(Is) > 0) {
        static_for_iseq_impl(std::forward<Lambda>(f), Is_t());
      }
    }
  } else {
    f(I_t{});
    if constexpr (sizeof...(Is) > 0) {
      static_for_iseq_impl(std::forward<Lambda>(f), Is_t());
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

}  // namespace cbr

#endif  // CBR_UTILS__STATIC_FOR_HPP_

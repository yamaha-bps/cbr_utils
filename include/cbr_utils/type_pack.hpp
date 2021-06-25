// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__TYPE_PACK_HPP_
#define CBR_UTILS__TYPE_PACK_HPP_

#include <tuple>
#include <type_traits>
#include <utility>

#include "integer_sequence.hpp"
#include "static_for.hpp"

namespace cbr
{

/***************************************************************************
 * \brief Type pack Concatenation
 ***************************************************************************/
namespace detail
{
template<typename ...>
struct _typepack_cat;

template<template<typename ...> typename T, typename ... Args>
struct _typepack_cat<T<Args...>>
{
  using type = T<Args ...>;
};

template<template<typename ...> typename T, typename ... Args1,
  typename ... Args2, typename ... Rem>
struct _typepack_cat<T<Args1...>, T<Args2...>, Rem...>
{
  using type = typename _typepack_cat<T<Args1 ..., Args2 ...>, Rem...>::type;
};
}  // namespace detail

template<typename ... Ts>
using typepack_cat = typename detail::_typepack_cat<Ts...>;

template<typename ... Ts>
using typepack_cat_t = typename typepack_cat<Ts...>::type;


/***************************************************************************
 * \brief Type pack duplicate
 ***************************************************************************/
namespace detail
{
template<typename T, size_t Idx>
using _typepack_dupli_ignore = T;

template<typename T, typename Lst>
struct _typepack_dupli {};

template<typename T, size_t ... Idx>
struct _typepack_dupli<T, std::index_sequence<Idx...>>
{
  using type = typename typepack_cat<_typepack_dupli_ignore<T, Idx>...>::type;
};

}  // namespace detail

template<typename T, size_t N>
using typepack_dupli = typename detail::_typepack_dupli<T, std::make_index_sequence<N>>;

template<typename T, size_t N>
using typepack_dupli_t = typename typepack_dupli<T, N>::type;


/***************************************************************************
 * \brief Type pack
 ***************************************************************************/
template<typename ... Args>
struct TypePack;

namespace detail
{
template<typename Pack, typename ISeq>
struct _subset_impl;

template<typename T1, typename T2>
struct _reverse_impl;

template<typename T, size_t ... Idx>
struct _subset_impl<T, std::index_sequence<Idx...>>
{
  using type = TypePack<typename T::template type<Idx>...>;
};

template<typename ... Pack>
struct _reverse_impl<TypePack<Pack...>, TypePack<>>
{
  using type = TypePack<Pack...>;
};

template<typename First, typename ... Pack1, typename ... Pack2>
struct _reverse_impl<TypePack<Pack1...>, TypePack<First, Pack2...>>
{
  using type = typename _reverse_impl<TypePack<First, Pack1...>, TypePack<Pack2...>>::type;
};

template<template<typename> typename T, typename ...>
struct member_v_impl;

template<template<typename> typename T>
struct member_v_impl<T>
{
  using type = std::index_sequence<>;
};

template<template<typename> typename T, typename ... Args>
struct member_v_impl
{
  using type = std::integer_sequence<
    std::common_type_t<decltype(T<Args>::value)...>, T<Args>::value...
  >;
};

}  // namespace detail


template<typename ... Args>
struct TypePack
{
private:
  using this_t = TypePack<Args...>;

public:
  static constexpr std::size_t size = sizeof...(Args);

  using tuple = std::tuple<Args...>;

  template<std::size_t N>
  using type = typename std::tuple_element_t<N, tuple>;

  template<template<typename ...> typename T, typename ... Ps>
  using apply = TypePack<T<Args, Ps...>...>;

  template<template<typename ...> typename T, typename ... Ps>
  using apply_prefixed = TypePack<T<Ps..., Args>...>;

  template<template<typename ...> typename T, typename ... Ts>
  using unpack = T<Args..., Ts...>;

  template<template<typename ...> typename T, typename ... Ts>
  using unpack_prefixed = T<Ts..., Args...>;

  template<std::size_t N>
  using duplicate =
    typename detail::_typepack_dupli<this_t, std::make_index_sequence<N>>::type;

  template<typename ... Ts>
  using append = typename typepack_cat<this_t, TypePack<Ts...>>::type;

  template<typename ... Ts>
  using append_front = typename typepack_cat<TypePack<Ts...>, this_t>::type;

  template<template<typename ...> typename T, typename ... Ts>
  using member_t = TypePack<typename T<Args, Ts...>::type...>;

  template<template<typename> typename T>
  using member_v = typename detail::member_v_impl<T, Args...>::type;

  // TypePack subset from std::index_sequence
  template<typename ISeq>
  using subset = typename detail::_subset_impl<this_t, ISeq>::type;

  // reversed TypePack
  using reversed = typename detail::_reverse_impl<TypePack<>, this_t>::type;

  // TypePack of first N elements
  template<size_t N>
  using head = subset<typename std::make_index_sequence<N>>;

  // TypePack of last N elements
  template<size_t N>
  using tail = typename reversed::template head<N>::reversed;

  // strip reference and cv qualifiers from TypePack
  using decay = apply<std::decay_t>;

  template<typename T>
  static void loop(T && f)
  {
    if constexpr (size > 0) {
      static_for_aggregate(tuple{}, std::forward<T>(f));
    }
  }
};

}  // namespace cbr


#endif  // CBR_UTILS__TYPE_PACK_HPP_

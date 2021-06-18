// Copyright 2020 Yamaha Motor Corporation, USA
#ifndef CBR_UTILS__INTEGER_PACK_HPP_
#define CBR_UTILS__INTEGER_PACK_HPP_

#include <variant>
#include <array>
#include <type_traits>
#include <utility>

#include "integer_sequence.hpp"
#include "static_for.hpp"
#include "type_pack.hpp"
#include "type_traits.hpp"

namespace cbr
{

/***************************************************************************
 * \brief List of integers
 ***************************************************************************/
template<typename T, T ... Vs>
struct IntegerPack;

namespace detail
{

template<typename T, T ... Vs>
struct IntegerPackBuild;

template<typename Type, Type ... Vals>
struct IntegerPackBuild<std::integer_sequence<Type, Vals...>>
{
  using type = IntegerPack<Type, Vals...>;
};

template<typename T, T N>
struct IntegerPackBuild<T, N>
{
  using type = typename IntegerPackBuild<make_integer_sequence<T, N>>::type;
};

template<typename T, T First, T Last>
struct IntegerPackBuild<T, First, Last>
{
  using type = typename IntegerPackBuild<make_integer_sequence<T, First, Last>>::type;
};

template<typename Pack, typename ISeq>
struct _subset_impl_int;

template<typename Pack, std::size_t ... Idx>
struct _subset_impl_int<Pack, std::index_sequence<Idx...>>
{
  using type = IntegerPack<typename Pack::type, Pack::array[Idx] ...>;
};

template<typename T1, typename T2>
struct _reverse_impl_int;

template<typename Type, Type ... Vals>
struct _reverse_impl_int<IntegerPack<Type, Vals...>, IntegerPack<Type>>
{
  using type = IntegerPack<Type, Vals...>;
};

template<typename Type, Type First, Type ... Pack1, Type ... Pack2>
struct _reverse_impl_int<IntegerPack<Type, Pack1...>, IntegerPack<Type, First, Pack2...>>
{
  using type =
    typename _reverse_impl_int<IntegerPack<Type, First, Pack1...>,
      IntegerPack<Type, Pack2...>>::type;
};

template<typename ...>
struct _integerpack_cat;

template<template<typename T, T...> typename Pack, typename Type, Type ... Vals>
struct _integerpack_cat<Pack<Type, Vals...>>
{
  using type = Pack<Type, Vals...>;
};

template<
  template<typename T1, T1...> typename Pack,
  typename Type1, Type1 ... Vals1,
  typename Type2, Type2 ... Vals2,
  typename ... Rem>
struct _integerpack_cat<Pack<Type1, Vals1...>, Pack<Type2, Vals2...>, Rem...>
{
  using type = typename _integerpack_cat<
    Pack<std::common_type_t<Type1, Type2>, Vals1 ..., Vals2 ...>,
    Rem...>::type;
};

template<typename T, size_t Idx>
using _integerpack_dupli_ignore = T;

template<typename T, typename Lst>
struct _integerpack_dupli {};

template<typename T, size_t ... Idx>
struct _integerpack_dupli<T, std::index_sequence<Idx...>>
{
  using type = typename _integerpack_cat<_integerpack_dupli_ignore<T, Idx>...>::type;
};

}  // namespace detail

template<typename ... Ts>
using integerpack_cat = typename detail::_integerpack_cat<Ts...>;

template<typename ... Ts>
using integerpack_cat_t = typename integerpack_cat<Ts...>::type;

template<typename Type, Type ... Vals>
struct IntegerPack
{
private:
  using this_t = IntegerPack<Type, Vals...>;

public:
  static constexpr std::size_t size = sizeof...(Vals);
  using type = Type;

  using integer_sequence = std::integer_sequence<Type, Vals...>;

  constexpr static std::array<Type, size> array{Vals ...};
  constexpr static std::tuple tuple = std::make_tuple(Vals ...);

  template<std::size_t idx>
  constexpr static Type value() {return array[idx];}

  template<template<Type ...> typename T, Type ... Vs>
  using apply = TypePack<T<Vals, Vs...>...>;

  template<template<Type, typename ...> typename T, typename ... Ts>
  using apply_typed = TypePack<T<Vals, Ts...>...>;

  template<template<Type ...> typename T, Type ... Vs>
  using apply_prefixed = TypePack<T<Vs ..., Vals>...>;

  template<template<typename, Type...> typename T, typename T1>
  using apply_prefixed_typed = TypePack<T<T1, Vals>...>;

  template<template<typename, typename, Type...> typename T,
    typename T1, typename T2>
  using apply_prefixed_typed_2 = TypePack<T<T1, T2, Vals>...>;

  template<template<typename, typename, typename, Type...> typename T,
    typename T1, typename T2, typename T3>
  using apply_prefixed_typed_3 = TypePack<T<T1, T2, T3, Vals>...>;

  template<template<typename, typename, typename, typename, Type...> typename T,
    typename T1, typename T2, typename T3, typename T4>
  using apply_prefixed_typed_4 = TypePack<T<T1, T2, T3, T4, Vals>...>;

  template<template<typename, typename, typename, typename, typename, Type...> typename T,
    typename T1, typename T2, typename T3, typename T4, typename T5>
  using apply_prefixed_typed_5 = TypePack<T<T1, T2, T3, T4, T5, Vals>...>;

  template<template<Type ...> typename T, Type ... Vs>
  using unpack = T<Vals..., Vs...>;

  template<template<Type ...> typename T, Type ... Vs>
  using unpack_prefixed = T<Vs..., Vals...>;

  template<template<typename, Type ...> typename T, typename T1>
  using unpack_prefixed_typed = T<T1, Vals...>;

  template<template<typename, typename, Type ...> typename T,
    typename T1, typename T2>
  using unpack_prefixed_typed_2 = T<T1, T2, Vals...>;

  template<template<typename, typename, typename, Type ...> typename T,
    typename T1, typename T2, typename T3>
  using unpack_prefixed_typed_3 = T<T1, T2, T3, Vals...>;

  template<template<typename, typename, typename, typename, Type ...> typename T,
    typename T1, typename T2, typename T3, typename T4>
  using unpack_prefixed_typed_4 = T<T1, T2, T3, T4, Vals...>;

  template<template<typename, typename, typename, typename, typename, Type ...> typename T,
    typename T1, typename T2, typename T3, typename T4, typename T5>
  using unpack_prefixed_typed_5 = T<T1, T2, T3, T4, T5, Vals...>;

  template<std::size_t N>
  using duplicate =
    typename detail::_integerpack_dupli<this_t, std::make_index_sequence<N>>::type;

  template<Type ... Vs>
  using append = typename detail::_integerpack_cat<this_t, IntegerPack<Type, Vs...>>::type;

  template<Type ... Vs>
  using append_front = typename detail::_integerpack_cat<IntegerPack<Type, Vs...>, this_t>::type;

  template<typename ISeq>
  using subset = typename detail::_subset_impl_int<this_t, ISeq>::type;

  // reversed TypePack
  using reversed = typename detail::_reverse_impl_int<IntegerPack<Type>, this_t>::type;

  template<size_t N>
  using head = typename detail::_subset_impl_int<this_t, std::make_index_sequence<N>>::type;

  template<size_t N>
  using tail = typename reversed::template head<N>::reversed;

  template<typename T>
  static void loop(T && f)
  {
    if constexpr (size > 0) {
      detail::static_for_iseq_impl(std::forward<T>(f), integer_sequence{});
    }
  }
};

template<typename ISeq>
using toIntegerPack = typename detail::IntegerPackBuild<ISeq>::type;

template<typename T, T ... Vs>
using makeIntegerPack = typename detail::IntegerPackBuild<T, Vs...>::type;

template<std::size_t ... Vs>
using IndexPack = IntegerPack<std::size_t, Vs...>;

template<typename ISeq,
  std::enable_if_t<std::is_same_v<typename ISeq::value_type, std::size_t>, int> = 0>
using toIndexPack = typename detail::IntegerPackBuild<ISeq>::type;

template<std::size_t ... Vs>
using makeIndexPack = typename detail::IntegerPackBuild<std::size_t, Vs...>::type;

template<template<std::size_t> typename T, std::size_t ... Vs>
using IndexPackVariant =
  typename IndexPack<Vs...>::template apply<T>::template unpack<std::variant>;

template<template<std::size_t> typename T, std::size_t ... Vs>
using makeIndexPackVariant =
  typename makeIndexPack<Vs...>::template apply<T>::template unpack<std::variant>;

template<template<std::size_t> typename T, std::size_t ... Vs>
using IndexPackVariantMonostate =
  typename IndexPack<Vs...>::template apply<T>::template unpack_prefixed<std::variant,
    std::monostate>;

template<template<std::size_t> typename T, std::size_t ... Vs>
using makeIndexPackVariantMonostate =
  typename makeIndexPack<Vs...>::template apply<T>::template unpack_prefixed<std::variant,
    std::monostate>;

}  // namespace cbr

#endif  // CBR_UTILS__INTEGER_PACK_HPP_

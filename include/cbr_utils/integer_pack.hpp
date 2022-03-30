// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

/** @file */

#ifndef CBR_UTILS__INTEGER_PACK_HPP_
#define CBR_UTILS__INTEGER_PACK_HPP_

#include <array>
#include <type_traits>
#include <utility>
#include <variant>

#include "integer_sequence.hpp"
#include "static_for.hpp"
#include "type_pack.hpp"
#include "type_traits.hpp"

namespace cbr {

/// @cond

template<typename T, T... Vs>
struct IntegerPack;

namespace detail {

template<typename T, T... Vs>
struct IntegerPackBuild;

template<typename Type, Type... Vals>
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

template<typename Pack, std::size_t... Idx>
struct _subset_impl_int<Pack, std::index_sequence<Idx...>>
{
  using type = IntegerPack<typename Pack::type, Pack::array[Idx]...>;
};

template<typename T1, typename T2>
struct _reverse_impl_int;

template<typename Type, Type... Vals>
struct _reverse_impl_int<IntegerPack<Type, Vals...>, IntegerPack<Type>>
{
  using type = IntegerPack<Type, Vals...>;
};

template<typename Type, Type First, Type... Pack1, Type... Pack2>
struct _reverse_impl_int<IntegerPack<Type, Pack1...>, IntegerPack<Type, First, Pack2...>>
{
  using type = typename _reverse_impl_int<IntegerPack<Type, First, Pack1...>,
    IntegerPack<Type, Pack2...>>::type;
};

template<typename...>
struct _integerpack_cat;

template<template<typename T, T...> typename Pack, typename Type, Type... Vals>
struct _integerpack_cat<Pack<Type, Vals...>>
{
  using type = Pack<Type, Vals...>;
};

template<template<typename T1, T1...> typename Pack,
  typename Type1,
  Type1... Vals1,
  typename Type2,
  Type2... Vals2,
  typename... Rem>
struct _integerpack_cat<Pack<Type1, Vals1...>, Pack<Type2, Vals2...>, Rem...>
{
  using type = typename _integerpack_cat<Pack<std::common_type_t<Type1, Type2>, Vals1..., Vals2...>,
    Rem...>::type;
};

template<typename T, size_t Idx>
using _integerpack_dupli_ignore = T;

template<typename T, typename Lst>
struct _integerpack_dupli
{};

template<typename T, size_t... Idx>
struct _integerpack_dupli<T, std::index_sequence<Idx...>>
{
  using type = typename _integerpack_cat<_integerpack_dupli_ignore<T, Idx>...>::type;
};

}  // namespace detail

template<typename... Ts>
using integerpack_cat = typename detail::_integerpack_cat<Ts...>;

/// @endcond

/**
 * @brief Concatenate multiple IntegerPack into one.
 *
 * @tparam Ts Integer packs to concatenate.
 */
template<typename... Ts>
using integerpack_cat_t = typename integerpack_cat<Ts...>::type;

/**
 * @brief Compile time pack of integers.
 * @details Similar to std::integer_sequence but with more functionalities.
 *
 * @tparam Type An integer type to use for the elements of the pack.
 * @tparam Vals A non-type parameter pack representing the pack.
 */
template<typename Type, Type... Vals>
struct IntegerPack
{
protected:
  /**
   * @brief Type of the pack.
   */
  using this_t = IntegerPack<Type, Vals...>;

public:
  /**
   * @brief Size of the pack.
   */
  static constexpr std::size_t size = sizeof...(Vals);

  /**
   * @brief Type of the elements of the packt.
   */
  using type = Type;

  /**
   * @brief std::integer_sequence representation of the pack.
   */
  using integer_sequence = std::integer_sequence<Type, Vals...>;

  /**
   * @brief std::array representation of the pack.
   */
  constexpr static std::array<Type, size> array{Vals...};

  /**
   * @brief std::tuple representation of the pack.
   */
  constexpr static std::tuple tuple = std::make_tuple(Vals...);

  /**
   * @brief Get list element at given index.
   *
   * @tparam idx Index of the element to return.
   * @return Element of the list at specified index.
   */
  template<std::size_t idx>
  constexpr static Type value()
  {
    return array[idx];
  }

  /**
   * @brief Application of a given templated type over each element of the pack.
   * @details Result is a typepack.
   *
   * For example:
   * ```
   * template<int i, int j, int k>
   * struct S{};
   *
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::apply<S,9,8>;
   * ```
   *
   * Then T2 == TypePack<S<0,9,8>,S<1,9,8>,S<2,9,8>>.
   *
   * @tparam T Templated type to use for application.
   * @tparam Vs Optional values to append to T's template parameters.
   */
  template<template<Type...> typename T, Type... Vs>
  using apply = TypePack<T<Vals, Vs...>...>;

  /**
   * @brief Application of a given templated type over each element of the pack.
   * @details Result is a typepack.
   *
   * For example:
   * ```
   * template<int i, class T1, class T2>
   * struct S{};
   *
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::apply_typed<S,double,float>;
   * ```
   *
   * Then T2 == TypePack<S<0,double,float>,S<1,double,float>,S<2,double,float>>.
   *
   * @tparam T Templated type to use for application.
   * @tparam Ts Optional types to append to the type's template parameters.
   */
  template<template<Type, typename...> typename T, typename... Ts>
  using apply_typed = TypePack<T<Vals, Ts...>...>;

  /**
   * @brief Application of a given templated type over each element of the pack.
   * @details Result is a typepack.
   *
   * For example:
   * ```
   * template<int i, int j, int k>
   * struct S{};
   *
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::apply_prefixed<S,9,8>;
   * ```
   *
   * Then T2 == TypePack<S<9,8,0>,S<9,8,1>,S<9,8,2>>.
   *
   * @tparam T Templated type to use for application.
   * @tparam Vs Optional values to prepend to the type's template parameters.
   */
  template<template<Type...> typename T, Type... Vs>
  using apply_prefixed = TypePack<T<Vs..., Vals>...>;

  /**
   * @brief Application of a given templated type over each element of the pack.
   * @details Result is a typepack.
   *
   * For example:
   * ```
   * template<class T1, int i>
   * struct S{};
   *
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::apply_prefixed_typed<S,double>;
   * ```
   *
   * Then T2 == TypePack<S<double,0>,S<double,1>,S<double,2>>.
   *
   * @tparam T Templated type to use for application.
   * @tparam T1 Optional type to prepend to the type's template parameters.
   */
  template<template<typename, Type...> typename T, typename T1>
  using apply_prefixed_typed = TypePack<T<T1, Vals>...>;

  /**
   * @brief Same as apply_prefixed_typed but with 2 typed template parameters.
   */
  template<template<typename, typename, Type...> typename T, typename T1, typename T2>
  using apply_prefixed_typed_2 = TypePack<T<T1, T2, Vals>...>;

  /**
   * @brief Same as apply_prefixed_typed but with 3 typed template parameters.
   */
  template<template<typename, typename, typename, Type...> typename T,
    typename T1,
    typename T2,
    typename T3>
  using apply_prefixed_typed_3 = TypePack<T<T1, T2, T3, Vals>...>;

  /**
   * @brief Same as apply_prefixed_typed but with 4 typed template parameters.
   */
  template<template<typename, typename, typename, typename, Type...> typename T,
    typename T1,
    typename T2,
    typename T3,
    typename T4>
  using apply_prefixed_typed_4 = TypePack<T<T1, T2, T3, T4, Vals>...>;

  /**
   * @brief Same as apply_prefixed_typed but with 5 typed template parameters.
   */
  template<template<typename, typename, typename, typename, typename, Type...> typename T,
    typename T1,
    typename T2,
    typename T3,
    typename T4,
    typename T5>
  using apply_prefixed_typed_5 = TypePack<T<T1, T2, T3, T4, T5, Vals>...>;

  /**
   * @brief Unpack elements of the pack into the template parameters of a given type.
   * @details For example:
   * ```
   * template<int... vs>
   * struct S{};
   *
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::unpack<S, 5, 6>;
   * ```
   *
   * Then T2 == S<0,1,2,5,6>.
   *
   * @tparam T Templated type to use for unpacking.
   * @tparam Vs Optional values to append to the type's template parameters.
   */
  template<template<Type...> typename T, Type... Vs>
  using unpack = T<Vals..., Vs...>;

  /**
   * @brief Unpack elements of the pack into the template parameters of a given type.
   * @details For example:
   * ```
   * template<int... vs>
   * struct S{};
   *
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::unpack_prefixed<S, 5, 6>;
   * ```
   *
   * Then T2 == S<5,6, 0,1,2>.
   *
   * @tparam T Templated type to use for unpacking.
   * @tparam Vs Optional values to prepend to the type's template parameters.
   */
  template<template<Type...> typename T, Type... Vs>
  using unpack_prefixed = T<Vs..., Vals...>;

  /**
   * @brief Unpack elements of the pack into the template parameters of a given type.
   * @details For example:
   * ```
   * template<class T, int... vs>
   * struct S{};
   *
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::unpack_prefixed_typed<S, double>;
   * ```
   *
   * Then T2 == S<double,0,1,2>.
   *
   * @tparam T Templated type to use for unpacking.
   * @tparam T1 Optional type to prepend to the type's template parameters.
   */
  template<template<typename, Type...> typename T, typename T1>
  using unpack_prefixed_typed = T<T1, Vals...>;

  /**
   * @brief Same as apply_prefixed_typed but with 2 typed template parameters.
   */
  template<template<typename, typename, Type...> typename T, typename T1, typename T2>
  using unpack_prefixed_typed_2 = T<T1, T2, Vals...>;

  /**
   * @brief Same as apply_prefixed_typed but with 3 typed template parameters.
   */
  template<template<typename, typename, typename, Type...> typename T,
    typename T1,
    typename T2,
    typename T3>
  using unpack_prefixed_typed_3 = T<T1, T2, T3, Vals...>;

  /**
   * @brief Same as apply_prefixed_typed but with 4 typed template parameters.
   */
  template<template<typename, typename, typename, typename, Type...> typename T,
    typename T1,
    typename T2,
    typename T3,
    typename T4>
  using unpack_prefixed_typed_4 = T<T1, T2, T3, T4, Vals...>;

  /**
   * @brief Same as apply_prefixed_typed but with 5 typed template parameters.
   */
  template<template<typename, typename, typename, typename, typename, Type...> typename T,
    typename T1,
    typename T2,
    typename T3,
    typename T4,
    typename T5>
  using unpack_prefixed_typed_5 = T<T1, T2, T3, T4, T5, Vals...>;

  /**
   * @brief Duplicate the pack multiple times.
   * @details Result is another IntegerPack.
   *
   * Example:
   * ```
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::duplicate<3>;
   * ```
   *
   * Then T2 == IntegerPack<int,0,1,2,0,1,2,0,1,2>.
   *
   * @tparam N Number of times to duplicate the pack.
   */
  template<std::size_t N>
  using duplicate = typename detail::_integerpack_dupli<this_t, std::make_index_sequence<N>>::type;

  /**
   * @brief Append values to the pack.
   * @details Result is another IntegerPack.
   *
   * Example:
   * ```
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::append<3, 4>;
   * ```
   *
   * Then T2 == IntegerPack<int,0,1,2,3,4>.
   *
   * @tparam Vs Values to append to the pack.
   */
  template<Type... Vs>
  using append = typename detail::_integerpack_cat<this_t, IntegerPack<Type, Vs...>>::type;

  /**
   * @brief Prepend values to the pack.
   * @details Result is another IntegerPack.
   *
   * Example:
   * ```
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::append_front<3, 4>;
   * ```
   *
   * Then T2 == IntegerPack<int,3,4,0,1,2>.
   *
   * @tparam Vs Values to prepend to the pack.
   */
  template<Type... Vs>
  using append_front = typename detail::_integerpack_cat<IntegerPack<Type, Vs...>, this_t>::type;

  /**
   * @brief Extract subset of the pack.
   * @details Result is another IntegerPack.
   *
   * Example:
   * ```
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::subset<std::index_sequence<0, 2>>;
   * ```
   *
   * Then T2 == IntegerPack<int,0,2>.
   *
   * @tparam Vs Values to prepend to the pack.
   */
  template<typename ISeq>
  using subset = typename detail::_subset_impl_int<this_t, ISeq>::type;

  /**
   * @brief Reverse order of elements in the pack.
   * @details Result is another IntegerPack.
   *
   * Example:
   * ```
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::reversed;
   * ```
   *
   * Then T2 == IntegerPack<int,2,1,0>.
   *
   * @tparam ISeq An std::index_sequence of indexes of elements that will compose the subset.
   */
  using reversed = typename detail::_reverse_impl_int<IntegerPack<Type>, this_t>::type;

  /**
   * @brief Extract first elements of the pack.
   * @details Result is another IntegerPack.
   *
   * Example:
   * ```
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::head<2>;
   * ```
   *
   * Then T2 == IntegerPack<int,0,1>.
   *
   * @tparam N Number of elements to extract.
   */
  template<size_t N>
  using head = typename detail::_subset_impl_int<this_t, std::make_index_sequence<N>>::type;

  /**
   * @brief Extract last elements of the pack.
   * @details Result is another IntegerPack.
   *
   * Example:
   * ```
   * using T = IntegerPack<int, 0, 1, 2>;
   * using T2 = T::tail<2>;
   * ```
   *
   * Then T2 == IntegerPack<int,1,2>.
   *
   * @tparam N Number of elements to extract.
   */
  template<size_t N>
  using tail = typename reversed::template head<N>::reversed;

  /**
   * @brief Loop over the elements of the pack.
   * @details Example:
   * ```
   * using T = IntegerPack<int, 0, 1, 2>;
   * T::loop([]{const auto i}{std::cout << i.value << std::endl;});
   * ```
   *
   * @tparam T Type of the callable.
   */
  template<typename T>
  static void loop(T && f)
  {
    if constexpr (size > 0) {
      detail::static_for_iseq_impl(std::forward<T>(f), integer_sequence{});
    }
  }
};

/**
 * @brief Create integer pack from std::integer_sequence
 */
template<typename ISeq>
using toIntegerPack = typename detail::IntegerPackBuild<ISeq>::type;

/**
 * @brief Create integer pack.
 * @details - If Vs is empty, equivalent to toIntegerPack
 * - If Vs is 2 elements, create an integer pack of all values between this 2 elements
 * (2 elements included).
 * - If Vs is 1 element, create an integer pack from 0 to Vs (Vs excluded).
 *
 * @tparam T Type of the elements of the integer pack, or std::integer_sequence.
 * @tparam Vs Construction parameters for the integer pack.
 */
template<typename T, T... Vs>
using makeIntegerPack = typename detail::IntegerPackBuild<T, Vs...>::type;

/**
 * @brief Alias for IntegerPack with type std::size_t
 */
template<std::size_t... Vs>
using IndexPack = IntegerPack<std::size_t, Vs...>;

/**
 * @brief Same as toIntegerPack but with type std::size_t
 */
template<typename ISeq,
  std::enable_if_t<std::is_same_v<typename ISeq::value_type, std::size_t>, int> = 0>
using toIndexPack = typename detail::IntegerPackBuild<ISeq>::type;

/**
 * @brief Same as makeIntegerPack but with type std::size_t
 */
template<std::size_t... Vs>
using makeIndexPack = typename detail::IntegerPackBuild<std::size_t, Vs...>::type;

/**
 * @brief Create an std::variant out of a templated type and values.
 * @details Create an index pack from values, apply type, and unpack into variant .
 *
 * Example:
 * ```
 * template<std::size_t i>
 * struct S{};
 *
 * using T = IndexPackVariant<S,0,1,2>;
 * ```
 *
 * Then T = std::variant<S<0>,S<1>,S<2>>
 *
 * @tparam T Templated type to make a variant out off
 * @tparam Vs Values to use to create the variant.
 */
template<template<std::size_t> typename T, std::size_t... Vs>
using IndexPackVariant =
  typename IndexPack<Vs...>::template apply<T>::template unpack<std::variant>;

/**
 * @brief Create an std::variant out of a templated type and values.
 * @details Same as IndexPackVariant but uses makeIndexPack to create the index pack.
 *
 * Example:
 * ```
 * template<std::size_t i>
 * struct S{};
 *
 * using T = makeIndexPackVariant<S,0,3>;
 * ```
 *
 * Then T = std::variant<S<0>,S<1>,S<2>,S<3>>
 *
 * @tparam T Templated type to make a variant out off
 * @tparam Vs Values to use to create the variant.
 */
template<template<std::size_t> typename T, std::size_t... Vs>
using makeIndexPackVariant =
  typename makeIndexPack<Vs...>::template apply<T>::template unpack<std::variant>;

/**
 * @brief Same as IndexPackVariant but with a variant starting with a std::monostate.
 */
template<template<std::size_t> typename T, std::size_t... Vs>
using IndexPackVariantMonostate =
  typename IndexPack<Vs...>::template apply<T>::template unpack_prefixed<std::variant,
    std::monostate>;

/**
 * @brief Same as makeIndexPackVariant but with a variant starting with a std::monostate.
 */
template<template<std::size_t> typename T, std::size_t... Vs>
using makeIndexPackVariantMonostate =
  typename makeIndexPack<Vs...>::template apply<T>::template unpack_prefixed<std::variant,
    std::monostate>;

}  // namespace cbr

#endif  // CBR_UTILS__INTEGER_PACK_HPP_

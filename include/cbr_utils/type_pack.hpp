// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

/** @file */

#ifndef CBR_UTILS__TYPE_PACK_HPP_
#define CBR_UTILS__TYPE_PACK_HPP_

#include <tuple>
#include <type_traits>
#include <utility>

#include "integer_sequence.hpp"
#include "static_for.hpp"

namespace cbr {

/// @cond

/***************************************************************************
 * \brief Type pack Concatenation
 ***************************************************************************/
namespace detail {
template<typename...>
struct _typepack_cat;

template<template<typename...> typename T, typename... Args>
struct _typepack_cat<T<Args...>>
{
  using type = T<Args...>;
};

template<template<typename...> typename T, typename... Args1, typename... Args2, typename... Rem>
struct _typepack_cat<T<Args1...>, T<Args2...>, Rem...>
{
  using type = typename _typepack_cat<T<Args1..., Args2...>, Rem...>::type;
};
}  // namespace detail

template<typename... Ts>
using typepack_cat = typename detail::_typepack_cat<Ts...>;

/// @endcond

/**
 * @brief Concatenate multiple TypePack into one.
 *
 * @tparam Ts Type packs to concatenate.
 */
template<typename... Ts>
using typepack_cat_t = typename typepack_cat<Ts...>::type;

/// @cond
namespace detail {
template<typename T, std::size_t Idx>
using _typepack_dupli_ignore = T;

template<typename T, typename Lst>
struct _typepack_dupli
{};

template<typename T, std::size_t... Idx>
struct _typepack_dupli<T, std::index_sequence<Idx...>>
{
  using type = typename typepack_cat<_typepack_dupli_ignore<T, Idx>...>::type;
};

}  // namespace detail

template<typename T, std::size_t N>
using typepack_dupli = typename detail::_typepack_dupli<T, std::make_index_sequence<N>>;

/// @endcond

/**
 * @brief Duplicate TypePack multiple times.
 *
 * @tparam T Type pack to duplicate.
 * @tparam N Number of time to duplicate the pack.
 */
template<typename T, std::size_t N>
using typepack_dupli_t = typename typepack_dupli<T, N>::type;

/***************************************************************************
 * \brief Type pack
 ***************************************************************************/
/// @cond
template<typename... Args>
struct TypePack;

namespace detail {
template<typename Pack, typename ISeq>
struct _subset_impl;

template<typename T1, typename T2>
struct _reverse_impl;

template<typename T, std::size_t... Idx>
struct _subset_impl<T, std::index_sequence<Idx...>>
{
  using type = TypePack<typename T::template type<Idx>...>;
};

template<typename... Pack>
struct _reverse_impl<TypePack<Pack...>, TypePack<>>
{
  using type = TypePack<Pack...>;
};

template<typename First, typename... Pack1, typename... Pack2>
struct _reverse_impl<TypePack<Pack1...>, TypePack<First, Pack2...>>
{
  using type = typename _reverse_impl<TypePack<First, Pack1...>, TypePack<Pack2...>>::type;
};

template<template<typename> typename T, typename...>
struct member_v_impl;

template<template<typename> typename T>
struct member_v_impl<T>
{
  using type = std::index_sequence<>;
};

template<template<typename> typename T, typename... Args>
struct member_v_impl
{
  using type =
    std::integer_sequence<std::common_type_t<decltype(T<Args>::value)...>, T<Args>::value...>;
};

}  // namespace detail

/// @endcond

/**
 * @brief Utility to manipulate a list of types.
 * @details Utility created to manipulate typed template parameter packs.
 *
 * @tparam Args List of types constituting the pack.
 */
template<typename... Args>
struct TypePack
{
private:
  /**
   * @brief Type of the pack.
   */
  using this_t = TypePack<Args...>;

public:
  /**
   * @brief Size of the pack.
   */
  static constexpr std::size_t size = sizeof...(Args);

  /**
   * @brief std::tuple of the elements of the packs.
   */
  using tuple = std::tuple<Args...>;

  /**
   * @brief Get pack type at given index.
   *
   * @tparam idx Index of the type to return.
   */
  template<std::size_t idx>
  using type = typename std::tuple_element_t<idx, tuple>;

  /**
   * @brief Application of a given templated type over each element of the pack.
   * @details Result is another TypePack.
   *
   * For example:
   * ```
   * template<class T>
   * struct S{};
   *
   * using T = TypePack<int, double, float>;
   * using T2 = T::apply<S,char,void*>;
   * ```
   *
   * Then T2 == TypePack<S<int,char,void*>,S<double,char,void*>,S<float,char,void*>>.
   *
   * @tparam T Templated type to use for application.
   * @tparam Ts Optional types to append to T's template parameters.
   */
  template<template<typename...> typename T, typename... Ts>
  using apply = TypePack<T<Args, Ts...>...>;

  /**
   * @brief Application of a given templated type over each element of the pack.
   * @details Result is another TypePack.
   *
   * For example:
   * ```
   * template<class T>
   * struct S{};
   *
   * using T = TypePack<int, double, float>;
   * using T2 = T::apply<S,char,void*>;
   * ```
   *
   * Then T2 == TypePack<S<char,void*,int>,S<char,void*,double>,S<char,void*,float>>.
   *
   * @tparam T Templated type to use for application.
   * @tparam Ts Optional types to prepend to T's template parameters.
   */
  template<template<typename...> typename T, typename... Ts>
  using apply_prefixed = TypePack<T<Ts..., Args>...>;

  /**
   * @brief Unpack elements of the pack into the template parameters of a given type.
   * @details For example:
   * ```
   * template<class T1, class T2, class T3>
   * struct S{};
   *
   * using T = TypePack<int, double>;
   * using T2 = T::unpack<S,char>;
   * ```
   *
   * Then T2 == S<int,double,char>.
   *
   * @tparam T Templated type to use for unpacking.
   * @tparam Ts Optional types to append to T's template parameters.
   */
  template<template<typename...> typename T, typename... Ts>
  using unpack = T<Args..., Ts...>;

  /**
   * @brief Unpack elements of the pack into the template parameters of a given type.
   * @details For example:
   * ```
   * template<class T1, class T2, class T3>
   * struct S{};
   *
   * using T = TypePack<int, double>;
   * using T2 = T::unpack<S,char>;
   * ```
   *
   * Then T2 == S<char,int,double>.
   *
   * @tparam T Templated type to use for unpacking.
   * @tparam Ts Optional types to prepend to T's template parameters.
   */
  template<template<typename...> typename T, typename... Ts>
  using unpack_prefixed = T<Ts..., Args...>;

  /**
   * @brief Duplicate the pack multiple times.
   * @details Result is another TypePack.
   *
   * Example:
   * ```
   * using T = TypePack<int, double>;
   * using T2 = T::duplicate<3>;
   * ```
   *
   * Then T2 == IntegerPack<int, double, int, double, int, double>.
   *
   * @tparam N Number of times to duplicate the pack.
   */
  template<std::size_t N>
  using duplicate = typename detail::_typepack_dupli<this_t, std::make_index_sequence<N>>::type;

  /**
   * @brief Append types to the pack.
   * @details Result is another TypePack.
   *
   * Example:
   * ```
   * using T = TypePack<int, double>;
   * using T2 = T::append<float,char>;
   * ```
   *
   * Then T2 == TypePack<int,double,float,char>>.
   *
   * @tparam Ts Types to append to the pack.
   */
  template<typename... Ts>
  using append = typename typepack_cat<this_t, TypePack<Ts...>>::type;

  template<typename... Ts>
  using append_front = typename typepack_cat<TypePack<Ts...>, this_t>::type;

  template<template<typename...> typename T, typename... Ts>
  using member_t = TypePack<typename T<Args, Ts...>::type...>;

  template<template<typename> typename T>
  using member_v = typename detail::member_v_impl<T, Args...>::type;

  // TypePack subset from std::index_sequence
  template<typename ISeq>
  using subset = typename detail::_subset_impl<this_t, ISeq>::type;

  // reversed TypePack
  using reversed = typename detail::_reverse_impl<TypePack<>, this_t>::type;

  // TypePack of first N elements
  template<std::size_t N>
  using head = subset<typename std::make_index_sequence<N>>;

  // TypePack of last N elements
  template<std::size_t N>
  using tail = typename reversed::template head<N>::reversed;

  // strip reference and cv qualifiers from TypePack
  using decay = apply<std::decay_t>;

  template<typename T>
  static void loop(T && f)
  {
    if constexpr (size > 0) { static_for_aggregate(tuple{}, std::forward<T>(f)); }
  }
};

}  // namespace cbr

#endif  // CBR_UTILS__TYPE_PACK_HPP_

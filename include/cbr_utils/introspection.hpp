// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

/** @file */

#ifndef CBR_UTILS__INTROSPECTION_HPP_
#define CBR_UTILS__INTROSPECTION_HPP_

#include <boost/hana/accessors.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/keys.hpp>
#include <boost/hana/size.hpp>

#include <tuple>
#include <type_traits>
#include <utility>

namespace cbr {
namespace detail {

template<typename Seq, std::size_t... Is>
auto copy_to_tuple_impl(Seq && s, std::index_sequence<Is...>)
{
  using Seq_t = std::decay_t<Seq>;
  using s_t   = decltype(s);

  static_assert(boost::hana::Struct<Seq_t>::value, "Input must be a boost::hana struct");

  constexpr auto accessors = boost::hana::accessors<Seq_t>();

  if constexpr (std::is_rvalue_reference_v<s_t>) {
    return std::make_tuple(
      std::move(boost::hana::second(accessors[boost::hana::size_c<Is>])(s))...);
  } else {
    return std::make_tuple(boost::hana::second(accessors[boost::hana::size_c<Is>])(s)...);
  }
}

template<typename Seq, std::size_t... Is>
auto bind_to_tuple_impl(Seq & s, std::index_sequence<Is...>)
{
  using Seq_t = std::decay_t<Seq>;

  static_assert(boost::hana::Struct<Seq_t>::value, "Input must be a boost::hana struct");

  constexpr auto accessors = boost::hana::accessors<Seq_t>();
  return std::tie(boost::hana::second(accessors[boost::hana::size_c<Is>])(s)...);
}

}  // namespace detail

/**
 * @brief copy the fields of a boost::hana::Struct into a tuple
 */
template<typename Seq>
auto copy_to_tuple(Seq && s)
{
  using Seq_t = std::decay_t<Seq>;

  static_assert(boost::hana::Struct<Seq_t>::value, "Input must be a boost::hana struct");

  return detail::copy_to_tuple_impl(
    std::forward<Seq>(s), std::make_index_sequence<decltype(boost::hana::length(s))::value>{});
}

/**
 * @brief ties the fields of a boost::hana::Struct to a tuple
 */
template<typename Seq>
auto bind_to_tuple(Seq & s)
{
  using Seq_t = std::decay_t<Seq>;

  static_assert(boost::hana::Struct<Seq_t>::value, "Input must be a boost::hana struct");

  return detail::bind_to_tuple_impl(
    s, std::make_index_sequence<decltype(boost::hana::length(s))::value>{});
}

}  // namespace cbr

#endif  // CBR_UTILS__INTROSPECTION_HPP_

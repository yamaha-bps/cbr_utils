// Copyright 2020 Yamaha Motor Corporation, USA
#ifndef CBR_UTILS__INTROSPECTION_HPP_
#define CBR_UTILS__INTROSPECTION_HPP_

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/fusion/adapted/struct/detail/extension.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/type_index.hpp>

#include <type_traits>
#include <utility>
#include <tuple>

namespace cbr
{
namespace detail
{

template<typename Seq, std::size_t... Is>
auto copy_to_tuple_impl(Seq && s, std::index_sequence<Is...>)
{
  using Seq_t = std::decay_t<Seq>;
  using s_t = decltype(s);

  using boost::fusion::traits::is_sequence;
  static_assert(is_sequence<Seq_t>::value, "Input must be a boost sequence");

  using boost::fusion::at_c;
  if constexpr (std::is_rvalue_reference_v<s_t>) {
    return std::make_tuple(std::move(at_c<Is>(s))...);
  } else {
    return std::make_tuple(at_c<Is>(s)...);
  }
}

template<typename Seq, std::size_t... Is>
auto bind_to_tuple_impl(Seq & s, std::index_sequence<Is...>)
{
  using Seq_t = std::decay_t<Seq>;

  using boost::fusion::traits::is_sequence;
  static_assert(is_sequence<Seq_t>::value, "Input must be a boost sequence");

  using boost::fusion::at_c;
  return std::tie(at_c<Is>(s)...);
}

}  // namespace detail

template<typename Seq>
auto copy_to_tuple(Seq && s)
{
  using Seq_t = std::decay_t<Seq>;

  using boost::fusion::traits::is_sequence;
  static_assert(is_sequence<Seq_t>::value, "Input must be a boost sequence");

  return detail::copy_to_tuple_impl(
    std::forward<Seq>(s),
    std::make_index_sequence<boost::fusion::result_of::size<Seq_t>::value>{});
}

template<typename Seq>
auto bind_to_tuple(Seq & s)
{
  using Seq_t = std::decay_t<Seq>;

  using boost::fusion::traits::is_sequence;
  static_assert(is_sequence<Seq_t>::value, "Input must be a boost sequence");

  return detail::bind_to_tuple_impl(
    s,
    std::make_index_sequence<boost::fusion::result_of::size<Seq_t>::value>{});
}

}  // namespace cbr

#endif  // CBR_UTILS__INTROSPECTION_HPP_

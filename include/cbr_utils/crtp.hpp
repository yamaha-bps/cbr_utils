// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

/** @file */

#ifndef CBR_UTILS__CRTP_HPP_
#define CBR_UTILS__CRTP_HPP_

namespace cbr {

/**
 * @brief crtp helper, small variation on https://www.fluentcpp.com/2017/05/19/crtp-helper/
 * @details Use as follow:
 * ```
 * template<class Derived>
 * class Base : crtp<Derived,Base<Derived>>;
 *
 * class Derived : Base<Derived>;
 * ```
 */
template<typename T, typename crtpType>
struct crtp
{
  constexpr T & underlying() { return static_cast<T &>(*this); }
  constexpr T const & underlying() const { return static_cast<T const &>(*this); }

private:
  crtp() = default;
  friend crtpType;
};

}  // namespace cbr

#endif  // CBR_UTILS__CRTP_HPP_

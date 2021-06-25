// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__CRTP_HPP_
#define CBR_UTILS__CRTP_HPP_

namespace cbr
{

/**
 * @brief crtp
 *
 */
template<typename T, typename crtpType>
struct crtp
{
  constexpr T & underlying() {return static_cast<T &>(*this);}
  constexpr T const & underlying() const {return static_cast<T const &>(*this);}

private:
  crtp() = default;
  friend crtpType;
};

}  // namespace cbr
#endif  // CBR_UTILS__CRTP_HPP_

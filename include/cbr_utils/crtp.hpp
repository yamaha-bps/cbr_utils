// Copyright 2020 Yamaha Motor Corporation, USA
#ifndef CBR_UTILS__CRTP_HPP_
#define CBR_UTILS__CRTP_HPP_

namespace cbr
{

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

// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_ros/blob/master/LICENSE

#ifndef CBR_UTILS__CYBER_ENUM_HPP_
#define CBR_UTILS__CYBER_ENUM_HPP_

#include <array>
#include <string>
#include <type_traits>

#include "crtp.hpp"

namespace cbr
{

/***************************************************************************
 * \brief Better enum class type
 ***************************************************************************/
template<typename T>
struct CyberEnum : crtp<T, CyberEnum<T>>
{
  constexpr CyberEnum() = default;
  constexpr CyberEnum(const CyberEnum &) = default;
  constexpr CyberEnum(CyberEnum &&) = default;
  constexpr CyberEnum & operator=(const CyberEnum &) = default;
  constexpr CyberEnum & operator=(CyberEnum &&) = default;
  ~CyberEnum() = default;

  constexpr CyberEnum(const int data)  // NOLINT
  : data_{data}
  {
    if (!is_valid()) {
      throw std::runtime_error("Enum value is invalid.");
    }
  }
  constexpr CyberEnum(const std::string_view data)  // NOLINT
  {
    bool found = check(data, data_);

    if (!found) {
      throw std::runtime_error("Enum value is invalid.");
    }
  }

  constexpr CyberEnum & operator=(const int data)  // NOLINT
  {
    if (!check(data)) {
      throw std::runtime_error("Enum value provided is invalid.");
    }
    data_ = data;
    return *this;
  }
  constexpr CyberEnum & operator=(const std::string_view data)  // NOLINT
  {
    int data_tmp;
    bool found = check(data, data_tmp);
    if (!found) {
      throw std::runtime_error("Enum value is invalid.");
    }
    data_ = data_tmp;
    return *this;
  }

  constexpr bool operator>(const std::string_view rhs) const noexcept
  {
    int data;
    bool found = check(rhs, data);
    if (!found) {
      return true;
    }
    return data_ > data;
  }
  constexpr bool operator<(const std::string_view rhs) const noexcept
  {
    int data;
    bool found = check(rhs, data);
    if (!found) {
      return true;
    }
    return data_ < data;
  }
  constexpr bool operator>=(const std::string_view rhs) const noexcept
  {
    int data;
    bool found = check(rhs, data);
    if (!found) {
      return true;
    }
    return data_ >= data;
  }
  constexpr bool operator<=(const std::string_view rhs) const noexcept
  {
    int data;
    bool found = check(rhs, data);
    if (!found) {
      return true;
    }
    return data_ <= data;
  }

  constexpr operator auto() const noexcept {return data_;}
  operator std::string() const noexcept {return c_str();}
  constexpr operator std::string_view() const noexcept {return c_str();}
  constexpr explicit operator const char *() const noexcept {return c_str();}

  constexpr const char * c_str() const noexcept
  {
    for (std::size_t i = 0; i < T::values.size(); i++) {
      if (T::values[i] == data_) {
        return T::names[i];
      }
    }
    return nullptr;
  }

  std::string string() const noexcept
  {
    return c_str();
  }

  constexpr std::string_view string_view() const noexcept
  {
    return c_str();
  }

  constexpr bool is_valid() const noexcept
  {
    return check(data_);
  }

  constexpr int get() const noexcept
  {
    return data_;
  }

  constexpr std::size_t operator[](const std::size_t i) const noexcept {return T::values[i];}
  constexpr const char * operator()(const std::size_t i) const noexcept{return T::names[i];}

  static constexpr bool check(const int data) noexcept
  {
    static_assert(
      T::values.size() == T::names.size(),
      "names and values arrays must have the same size");

    for (std::size_t i = 0; i < T::values.size(); i++) {
      if (T::values[i] == data) {
        return true;
      }
    }
    return false;
  }

  static constexpr bool check(const std::string_view dataStr, int & dataVal) noexcept
  {
    static_assert(
      T::values.size() == T::names.size(),
      "names and values arrays must have the same size");

    for (std::size_t i = 0; i < T::values.size(); i++) {
      if (T::names[i] == dataStr) {
        dataVal = T::values[i];
        return true;
      }
    }

    return false;
  }

private:
  int data_ = T::values[0];
};

// struct ExampleEnum : CyberEnum<ExampleEnum>
// {
//   using CyberEnum<ExampleEnum>::CyberEnum;
//   using CyberEnum<ExampleEnum>::operator=;

//   static constexpr int off = 0;
//   static constexpr int on = 1;

//   static constexpr std::array values = {0, 1};
//   static constexpr std::array names = {"off", "on"};
// };

template<typename T>
struct is_cyber_enum : public std::is_base_of<CyberEnum<T>, T>
{};

template<typename T>
inline constexpr bool is_cyber_enum_v = is_cyber_enum<T>::value;

}  // namespace cbr

#endif  // CBR_UTILS__CYBER_ENUM_HPP_

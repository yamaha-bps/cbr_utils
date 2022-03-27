// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

/** @file */

#ifndef CBR_UTILS__CYBER_ENUM_HPP_
#define CBR_UTILS__CYBER_ENUM_HPP_

#include <array>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace cbr {

/**
 * @brief Type unsafe enum
 * @details Enum that can be implicitly constructed/converted to and from integer and string_view.
 *
 * Example:
 *  ```
 * struct ExampleEnum : CyberEnum<ExampleEnum>
 * {
 *   using CyberEnum<ExampleEnum>::CyberEnum;
 *   using CyberEnum<ExampleEnum>::operator=;
 *
 *   static constexpr int off = 0;
 *   static constexpr int on = 1;
 *
 *   static constexpr std::array values = {0, 1};
 *   static constexpr std::array names = {"off", "on"};
 * };
 * ```
 */
template<typename T>
struct CyberEnum
{
  constexpr CyberEnum()                  = default;
  constexpr CyberEnum(const CyberEnum &) = default;
  constexpr CyberEnum(CyberEnum &&)      = default;
  constexpr CyberEnum & operator=(const CyberEnum &) = default;
  constexpr CyberEnum & operator=(CyberEnum &&) = default;
  ~CyberEnum()                                  = default;

  constexpr CyberEnum(const int data)  // NOLINT
      : data_{data}
  {
    if (!is_valid()) { throw std::runtime_error("Enum value is invalid."); }
  }
  constexpr CyberEnum(const std::string_view data)  // NOLINT
  {
    bool found = check(data, data_);

    if (!found) { throw std::runtime_error("Enum value is invalid."); }
  }

  /**
   * @brief Operator= from int
   */
  constexpr CyberEnum & operator=(const int data)  // NOLINT
  {
    if (!check(data)) { throw std::runtime_error("Enum value provided is invalid."); }
    data_ = data;
    return *this;
  }

  /**
   * @brief Operator= from string_view, string_view used due to read only purposes
   */
  constexpr CyberEnum & operator=(const std::string_view data)  // NOLINT
  {
    int data_tmp{};
    bool found = check(data, data_tmp);
    if (!found) { throw std::runtime_error("Enum value is invalid."); }
    data_ = data_tmp;
    return *this;
  }

  /**
   * @brief Operator> from string_view
   */
  constexpr bool operator>(const std::string_view rhs) const noexcept
  {
    int data{};
    bool found = check(rhs, data);
    if (!found) { return true; }
    return data_ > data;
  }

  /**
   * @brief Operator< from string_view
   */
  constexpr bool operator<(const std::string_view rhs) const noexcept
  {
    int data{};
    bool found = check(rhs, data);
    if (!found) { return true; }
    return data_ < data;
  }

  /**
   * @brief Operator>= from string_view
   */
  constexpr bool operator>=(const std::string_view rhs) const noexcept
  {
    int data{};
    bool found = check(rhs, data);
    if (!found) { return true; }
    return data_ >= data;
  }
  /**
   * @brief Operator<= from string_view
   */
  constexpr bool operator<=(const std::string_view rhs) const noexcept
  {
    int data{};
    bool found = check(rhs, data);
    if (!found) { return true; }
    return data_ <= data;
  }

  constexpr operator auto() const noexcept { return data_; }
  operator std::string() const noexcept { return c_str(); }
  constexpr operator std::string_view() const noexcept { return c_str(); }
  constexpr explicit operator const char *() const noexcept { return c_str(); }

  /**
   * @brief Returns a pointer to an array that contains a null-terminated sequence of characters
   */
  constexpr const char * c_str() const noexcept
  {
    for (std::size_t i = 0; i < T::values.size(); i++) {
      if (T::values[i] == data_) { return T::names[i]; }
    }
    return nullptr;
  }

  /**
   * @brief Returns the result of c_str() as type std::string
   */
  std::string string() const noexcept { return c_str(); }

  /**
   * @brief Returns the result of c_str() as type std::string_view
   */
  constexpr std::string_view string_view() const noexcept { return c_str(); }

  /**
   * @brief Returns true if data_ is a member of values array
   */
  constexpr bool is_valid() const noexcept { return check(data_); }

  /**
   * @brief Returns value of data_
   */
  constexpr int get() const noexcept { return data_; }

  constexpr std::size_t operator[](const std::size_t i) const noexcept { return T::values[i]; }
  constexpr const char * operator()(const std::size_t i) const noexcept { return T::names[i]; }

  /**
   * @brief Returns true if data is in values array
   */
  static constexpr bool check(const int data) noexcept
  {
    static_assert(
      T::values.size() == T::names.size(), "names and values arrays must have the same size");

    for (std::size_t i = 0; i < T::values.size(); i++) {
      if (T::values[i] == data) { return true; }
    }
    return false;
  }

  /**
   * @brief Returns true if dataStr is in names array, sets dataVal to corresponding element in
   *        value array
   */
  static constexpr bool check(const std::string_view dataStr, int & dataVal) noexcept
  {
    static_assert(
      T::values.size() == T::names.size(), "names and values arrays must have the same size");

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

template<typename T>
struct is_cyber_enum : public std::is_base_of<CyberEnum<T>, T>
{};

template<typename T>
inline constexpr bool is_cyber_enum_v = is_cyber_enum<T>::value;

}  // namespace cbr

#endif  // CBR_UTILS__CYBER_ENUM_HPP_

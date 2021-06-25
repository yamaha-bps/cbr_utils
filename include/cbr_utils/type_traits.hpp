// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__TYPE_TRAITS_HPP_
#define CBR_UTILS__TYPE_TRAITS_HPP_

#include <optional>
#include <string_view>
#include <variant>
#include <chrono>
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace cbr
{

/***************************************************************************
 * \brief Returns type of variable as string_view
 ***************************************************************************/
template<typename T>
constexpr auto type_name() noexcept
{
  std::string_view name = "Error: unsupported compiler", prefix, suffix;
#ifdef __clang__
  name = __PRETTY_FUNCTION__;
  prefix = "auto cbr::type_name() [T = ";
  suffix = "]";
#elif defined(__GNUC__)
  name = __PRETTY_FUNCTION__;
  prefix = "constexpr auto cbr::type_name() [with T = ";
  suffix = "]";
#elif defined(_MSC_VER)
  name = __FUNCSIG__;
  prefix = "auto __cdecl cbr::type_name<";
  suffix = ">(void) noexcept";
#endif
  name.remove_prefix(prefix.size());
  name.remove_suffix(suffix.size());
  return name;
}

/***************************************************************************
 * \brief Wrapper for static assertion over parameter pack
 ***************************************************************************/
template<template<typename> typename T, typename ... Ts>
constexpr bool static_assert_all()
{
  return std::conjunction<
    std::integral_constant<bool, T<Ts>::value>...
  >{};
}


/***************************************************************************
 * \brief Enum class type trait
 ***************************************************************************/
template<typename T, bool B = std::is_enum<T>::value>
struct is_scoped_enum : std::false_type {};

template<typename T>
struct is_scoped_enum<T, true>
  : std::integral_constant<bool,
    !std::is_convertible<T, typename std::underlying_type<T>::type>::value> {};

template<class T>
inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;


/***************************************************************************
 * \brief Specialization class type trait
 ***************************************************************************/
// Specialization
template<typename Test, template<typename ...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename ...> class Ref, typename ... Args>
struct is_specialization<Ref<Args...>, Ref>: std::true_type {};

template<typename Test, template<typename ...> class Ref>
inline constexpr bool is_specialization_v = is_specialization<Test, Ref>::value;

// std::vector
template<typename>
struct is_std_vector : std::false_type {};

template<typename T, typename ... Ts>
struct is_std_vector<std::vector<T, Ts...>>: std::true_type {};

template<typename T>
inline constexpr bool is_std_vector_v = is_std_vector<T>::value;

// std::array
template<typename, bool = false>
struct is_std_array : std::false_type {};

template<typename T, std::size_t S>
struct is_std_array<std::array<T, S>>: std::true_type {};

template<typename T>
inline constexpr bool is_std_array_v = is_std_array<T>::value;

// std::pair
template<typename>
struct is_std_pair : std::false_type {};

template<typename T1, typename T2>
struct is_std_pair<std::pair<T1, T2>>: std::true_type {};

template<typename T>
inline constexpr bool is_std_pair_v = is_std_pair<T>::value;

// std::tuple
template<typename>
struct is_std_tuple : std::false_type {};

template<typename ... Ts>
struct is_std_tuple<std::tuple<Ts...>>: std::true_type {};

template<typename T>
inline constexpr bool is_std_tuple_v = is_std_tuple<T>::value;

// std::variant
template<typename>
struct is_std_variant : std::false_type {};

template<typename ... Ts>
struct is_std_variant<std::variant<Ts...>>: std::true_type {};

template<typename T>
inline constexpr bool is_std_variant_v = is_std_variant<T>::value;

// std::shared_ptr
template<typename>
struct is_std_shared_ptr : std::false_type {};

template<typename T>
struct is_std_shared_ptr<std::shared_ptr<T>>: std::true_type {};

template<typename T>
inline constexpr bool is_std_shared_ptr_v = is_std_shared_ptr<T>::value;

// Chrono duration
template<typename T>
struct is_chrono_duration : std::false_type {};

template<typename R, typename P>
struct is_chrono_duration<std::chrono::duration<R, P>>: std::true_type {};

// std::unique_ptr
template<typename>
struct is_std_unique_ptr : std::false_type {};

template<typename T>
struct is_std_unique_ptr<std::unique_ptr<T>>: std::true_type {};

template<typename T>
inline constexpr bool is_std_unique_ptr_v = is_std_unique_ptr<T>::value;

// std::unique_ptr
template<typename>
struct is_std_optional : std::false_type {};

template<typename T>
struct is_std_optional<std::optional<T>>: std::true_type {};

template<typename T>
inline constexpr bool is_std_optional_v = is_std_optional<T>::value;

// False and true type
template<typename ...>
inline constexpr bool false_v = false;

template<typename ...>
inline constexpr bool true_v = true;

}  // namespace cbr

#endif  // CBR_UTILS__TYPE_TRAITS_HPP_

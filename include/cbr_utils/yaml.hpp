// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#ifndef CBR_UTILS__YAML_HPP_
#define CBR_UTILS__YAML_HPP_

#include <yaml-cpp/yaml.h>

#include <boost/hana/at_key.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/fuse.hpp>
#include <boost/hana/keys.hpp>

#include <optional>
#include <variant>
#include <string>
#include <tuple>
#include <utility>

#include "type_traits.hpp"
#include "static_for.hpp"

namespace YAML
{

/* Overload yaml-cpp encode and decode functions for several containers */
template<typename T>
struct convert
{
  static Node encode(const T & val)
  {
    if constexpr (boost::hana::Struct<T>::value) {
      Node ret;
      boost::hana::for_each(
        val, boost::hana::fuse(
          [&ret](auto key, const auto & value) {
            if constexpr (boost::hana::Struct<decltype(value)>::value) {
              ret[boost::hana::to<char const *>(key)] = YAML::Node(value);
            } else {
              ret[boost::hana::to<char const *>(key)] = value;
            }
          }));
      return ret;
    } else if constexpr (cbr::is_scoped_enum_v<T>) {  //NOLINT
      using UT = std::underlying_type_t<T>;
      return Node(static_cast<UT>(val));
    } else if constexpr (cbr::is_std_tuple_v<T>) {  //NOLINT
      Node yaml(NodeType::Sequence);
      cbr::static_for_index<std::tuple_size_v<T>>(
        [&](auto i) {
          yaml.push_back(std::get<i.value>(val));
        });
      return yaml;

    } else if constexpr (cbr::is_std_optional_v<T>) {  //NOLINT
      if (val.has_value()) {
        return Node(val.value());
      }
      return Node();
    } else {
      static_assert(::cbr::false_v<T>, "Unsupported type for YAML encoding.");
    }
  }

  static bool decode(const Node & yaml, T & val)
  {
    if constexpr (boost::hana::Struct<T>::value) {
      boost::hana::for_each(
        boost::hana::keys(val), [&](auto key) {
          using ValT = std::decay_t<decltype(boost::hana::at_key(val, key))>;

          char const * key_c = boost::hana::to<char const *>(key);
          if constexpr (cbr::is_std_optional_v<ValT>) {
            if (!yaml[key_c] || yaml[key_c].IsNull()) {
              boost::hana::at_key(val, key) = std::nullopt;
            } else {
              boost::hana::at_key(val, key) = yaml[key_c].template as<typename ValT::value_type>();
            }
          } else {
            boost::hana::at_key(val, key) = yaml[key_c].template as<ValT>();
          }
        });
      return true;
    } else if constexpr (cbr::is_scoped_enum_v<T>) {  //NOLINT
      try {
        using UT = std::underlying_type_t<T>;
        const auto v = yaml.as<UT>();
        if (v < static_cast<UT>(T::N_ELEMENTS)) {
          val = static_cast<T>(v);
        } else {
          val = T::UNKOWN;
        }
      } catch (const std::exception & e) {
        return false;
      }
      return true;
    } else if constexpr (cbr::is_std_tuple_v<T>) {  //NOLINT
      if (!yaml.IsSequence()) {
        return false;
      }
      if (yaml.size() != std::tuple_size_v<T>) {
        return false;
      }

      cbr::static_for_index<std::tuple_size_v<T>>(
        [&](auto i) {
          std::get<i.value>(val) = yaml[i].template as<std::tuple_element_t<i.value, T>>();
        });

      return true;

    } else if constexpr (cbr::is_std_optional_v<T>) {  //NOLINT
      if (!yaml || yaml.IsNull()) {
        val = std::nullopt;
        return true;
      }
      try {
        using UT = typename T::value_type;
        val = yaml.as<UT>();
      } catch (const std::exception & e) {
        return false;
      }
      return true;
    } else {
      static_assert(::cbr::false_v<T>, "Unsupported type for YAML decoding.");
    }
  }
};

template<>
struct as_if<void, void>
{
  explicit as_if(const Node & node_)
  : node(node_) {}
  const Node & node;

  std::variant<void *, bool, int, double, std::string> operator()() const
  {
    if (node.IsNull()) {
      return {};
    }

    std::variant<void *, bool, int, double, std::string> out;

    bool val_bool;
    if (convert<bool>::decode(node, val_bool)) {
      out.emplace<bool>(val_bool);
      return out;
    }

    int val_int;
    if (convert<int>::decode(node, val_int)) {
      out.emplace<int>(val_int);
      return out;
    }

    double val_double;
    if (convert<double>::decode(node, val_double)) {
      out.emplace<double>(val_double);
      return out;
    }

    std::string val_string;
    if (convert<std::string>::decode(node, val_string)) {
      out.emplace<std::string>(std::move(val_string));
      return out;
    }

    return out;
  }
};

template<typename T>
struct as_if<T, std::optional<T>>
{
  explicit as_if(const Node & node_)
  : node(node_) {}
  const Node & node;

  std::optional<T> operator()() const
  {
    std::optional<T> val;
    T t;
    if (!node.IsNull() && convert<T>::decode(node, t)) {
      val = std::move(t);
    }

    return val;
  }
};

// There is already a std::string partial specialisation, so we need a full specialisation here
template<>
struct as_if<std::string, std::optional<std::string>>
{
  explicit as_if(const Node & node_)
  : node(node_) {}
  const Node & node;

  std::optional<std::string> operator()() const
  {
    std::optional<std::string> val;
    std::string t;
    if (!node.IsNull() && convert<std::string>::decode(node, t)) {
      val = std::move(t);
    }

    return val;
  }
};

}  // namespace YAML

#endif  // CBR_UTILS__YAML_HPP_

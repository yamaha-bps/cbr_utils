// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#include <gtest/gtest.h>

#include <boost/hana/define_struct.hpp>
#include <boost/hana/adapt_struct.hpp>

#include <memory>
#include <string>
#include <vector>

#include "cbr_utils/yaml.hpp"

namespace cbr
{
namespace sub
{
struct SubParameters
{
  BOOST_HANA_DEFINE_STRUCT(
    SubParameters,
    (int, sub0),
    (float, sub1),
    (bool, sub2)
  );
};
}  // namespace sub

struct Parameters
{
  BOOST_HANA_DEFINE_STRUCT(
    Parameters,
    (std::string, param1),
    (int, param2),
    (double, param3),
    (sub::SubParameters, sub)
  );
};
}  // namespace cbr

TEST(Yaml, Basic)
{
  auto yaml = YAML::Load(
    "{param1: hello, param2: 2, param3: 1.01, sub: {sub0: 12312, sub1: -1.4, sub2: true}}"
  );

  auto config = yaml.as<cbr::Parameters>();

  ASSERT_NO_THROW(yaml.as<cbr::Parameters>());

  YAML::Node yaml2;
  ASSERT_NO_THROW(yaml2 = config);

  ASSERT_ANY_THROW(
    YAML::Load("{sub0: hallo, sub1: -1.4}").as<cbr::sub::SubParameters>()
  );

  ASSERT_ANY_THROW(
    YAML::Load("{sub0: 123}").as<cbr::sub::SubParameters>()
  );
}


struct InvisibleStruct
{
  int visible;
  double invisible{};  // needs default initialization
};

BOOST_HANA_ADAPT_STRUCT(
  InvisibleStruct,
  visible
);

TEST(Yaml, Invisible)
{
  auto yaml = YAML::Load("{visible: 5}");

  auto config = yaml.as<InvisibleStruct>();

  ASSERT_EQ(config.visible, 5);
  ASSERT_DOUBLE_EQ(config.invisible, double{});
}


struct VectorParameter
{
  BOOST_HANA_DEFINE_STRUCT(
    VectorParameter,
    (std::vector<int>, vector)
  );
};

TEST(Yaml, VectorType)
{
  auto yaml = YAML::Load("{vector: [1,2,3,4]}");
  auto config = yaml.as<VectorParameter>();

  ASSERT_EQ(config.vector.size(), size_t(4));
  ASSERT_EQ(config.vector[0], 1);
  ASSERT_EQ(config.vector[1], 2);
  ASSERT_EQ(config.vector[2], 3);
  ASSERT_EQ(config.vector[3], 4);
}

struct OptionalStruct1
{
  std::optional<double> optional_1 = 1.;
  std::optional<double> optional_2 = 1.;
};

struct OptionalStruct2
{
  std::optional<double> optional_1;
  std::optional<double> optional_2;
};

BOOST_HANA_ADAPT_STRUCT(
  OptionalStruct1,
  optional_1,
  optional_2
);

BOOST_HANA_ADAPT_STRUCT(
  OptionalStruct2,
  optional_1,
  optional_2
);

TEST(Yaml, Optional)
{
  {
    auto yaml = YAML::Load("{optional_1: ~, optional_2: ~}");
    auto config = yaml.as<OptionalStruct1>();
    ASSERT_TRUE(!config.optional_1.has_value());
    ASSERT_TRUE(!config.optional_2.has_value());

    yaml = YAML::Load("{optional_1: 1., optional_2: ~}");
    config = yaml.as<OptionalStruct1>();
    ASSERT_TRUE(config.optional_1.has_value());
    ASSERT_DOUBLE_EQ(config.optional_1.value(), 1.);
    ASSERT_TRUE(!config.optional_2.has_value());

    yaml = YAML::Load("{optional_1: ~, optional_2: 2.}");
    config = yaml.as<OptionalStruct1>();
    ASSERT_TRUE(config.optional_2.has_value());
    ASSERT_DOUBLE_EQ(config.optional_2.value(), 2.);
    ASSERT_TRUE(!config.optional_1.has_value());

    yaml = YAML::Load("{optional_1: 1., optional_2: 2.}");
    config = yaml.as<OptionalStruct1>();
    ASSERT_TRUE(config.optional_1.has_value());
    ASSERT_DOUBLE_EQ(config.optional_1.value(), 1.);
    ASSERT_TRUE(config.optional_2.has_value());
    ASSERT_DOUBLE_EQ(config.optional_2.value(), 2.);

    yaml = YAML::Load("{optional_1: 1.}");
    config = yaml.as<OptionalStruct1>();
    ASSERT_TRUE(config.optional_1.has_value());
    ASSERT_DOUBLE_EQ(config.optional_1.value(), 1.);
    ASSERT_TRUE(!config.optional_2.has_value());

    yaml = YAML::Load("{optional_2: 2.}");
    config = yaml.as<OptionalStruct1>();
    ASSERT_TRUE(config.optional_2.has_value());
    ASSERT_DOUBLE_EQ(config.optional_2.value(), 2.);
    ASSERT_TRUE(!config.optional_1.has_value());
  }

  {
    auto yaml = YAML::Load("{optional_1: ~, optional_2: ~}");
    auto config = yaml.as<OptionalStruct2>();
    ASSERT_TRUE(!config.optional_1.has_value());
    ASSERT_TRUE(!config.optional_2.has_value());

    yaml = YAML::Load("{optional_1: 1., optional_2: ~}");
    config = yaml.as<OptionalStruct2>();
    ASSERT_TRUE(config.optional_1.has_value());
    ASSERT_DOUBLE_EQ(config.optional_1.value(), 1.);
    ASSERT_TRUE(!config.optional_2.has_value());

    yaml = YAML::Load("{optional_1: ~, optional_2: 2.}");
    config = yaml.as<OptionalStruct2>();
    ASSERT_TRUE(config.optional_2.has_value());
    ASSERT_DOUBLE_EQ(config.optional_2.value(), 2.);
    ASSERT_TRUE(!config.optional_1.has_value());

    yaml = YAML::Load("{optional_1: 1., optional_2: 2.}");
    config = yaml.as<OptionalStruct2>();
    ASSERT_TRUE(config.optional_1.has_value());
    ASSERT_DOUBLE_EQ(config.optional_1.value(), 1.);
    ASSERT_TRUE(config.optional_2.has_value());
    ASSERT_DOUBLE_EQ(config.optional_2.value(), 2.);

    yaml = YAML::Load("{optional_1: 1.}");
    config = yaml.as<OptionalStruct2>();
    ASSERT_TRUE(config.optional_1.has_value());
    ASSERT_DOUBLE_EQ(config.optional_1.value(), 1.);
    ASSERT_TRUE(!config.optional_2.has_value());

    yaml = YAML::Load("{optional_2: 2.}");
    config = yaml.as<OptionalStruct2>();
    ASSERT_TRUE(config.optional_2.has_value());
    ASSERT_DOUBLE_EQ(config.optional_2.value(), 2.);
    ASSERT_TRUE(!config.optional_1.has_value());
  }
}

// Copyright Yamaha 2021
// MIT License
// https://github.com/yamaha-bps/cbr_utils/blob/master/LICENSE

#include <gtest/gtest.h>

#include "cbr_utils/cyber_enum.hpp"

struct ExampleEnum : cbr::CyberEnum<ExampleEnum>
{
  using cbr::CyberEnum<ExampleEnum>::CyberEnum;
  using cbr::CyberEnum<ExampleEnum>::operator=;

  static constexpr int off = 0;
  static constexpr int on  = 1;

  static constexpr std::array values = {1, 2};
  static constexpr std::array names  = {"off", "on"};
};

TEST(CyberEnum, Basic)
{
  using namespace std::literals;

  constexpr ExampleEnum e0;
  static_assert(e0.get() == 1);

  constexpr ExampleEnum e1("off");
  constexpr ExampleEnum e11(1);
  static_assert(e1 == 1);
  static_assert(e11 == 1);

  constexpr ExampleEnum e2("on");
  constexpr ExampleEnum e22(2);
  static_assert(e2 == 2);
  static_assert(e22 == 2);

  ExampleEnum e3;

  ASSERT_ANY_THROW(ExampleEnum("test"));
  ASSERT_ANY_THROW(ExampleEnum(0));
  ASSERT_NO_THROW(ExampleEnum(1));
  ASSERT_NO_THROW(ExampleEnum("off"));
  ASSERT_NO_THROW(ExampleEnum(2));
  ASSERT_NO_THROW(ExampleEnum("on"));

  ASSERT_ANY_THROW(e3 = 0);
  ASSERT_ANY_THROW(e3 = "test");
  ASSERT_NO_THROW(e3 = 1);
  ASSERT_NO_THROW(e3 = "off");
  ASSERT_NO_THROW(e3 = 2);
  ASSERT_NO_THROW(e3 = "on");

  ASSERT_EQ(e3, 2);
  ASSERT_EQ(e3, "on"sv);

  ASSERT_NE(e3, 1);
  ASSERT_NE(e3, "off"sv);
}
